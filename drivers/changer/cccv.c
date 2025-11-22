
// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 brokestar233 <3765589194@qq.com>
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/freezer.h>

#include <linux/changer/cccv.h>

#define CHG_CTRL_VOTER "CHG_CTRL_VOTER"

static struct charging_control g_chg_ctrl;
struct chg_symbols *cs = NULL;
atomic_t all_init_done = ATOMIC_INIT(0);
bool is_conecting = false;
enum oplus_chg_protocol_type current_protocol_type = CHG_PROTOCOL_INVALID;

static unsigned int cccv_sleep_jiffies;

// 读取电池电压 (mV)
static int __maybe_unused read_battery_voltage(void)
{
    union mms_msg_data data;
    int rc;

    if (!g_chg_ctrl.gauge_topic) {
        pr_err("Gauge topic not available\n");
        return -ENODEV;
    }

    rc = cs->oplus_mms_get_item_data(g_chg_ctrl.gauge_topic, GAUGE_ITEM_VOL_MAX, &data, false);
    if (rc < 0) {
        pr_err("Failed to read battery voltage, rc=%d\n", rc);
        return rc;
    }

    return data.intval;
}

// 读取输入电流 (mA)
static int __maybe_unused read_input_current(void)
{
    union mms_msg_data data;
    int rc;

    if (!g_chg_ctrl.gauge_topic) {
        pr_err("Gauge topic not available\n");
        return -ENODEV;
    }

    rc = cs->oplus_mms_get_item_data(g_chg_ctrl.gauge_topic, GAUGE_ITEM_CURR, &data, false);
    if (rc < 0) {
        pr_err("Failed to read input current, rc=%d\n", rc);
        return rc;
    }

    return data.intval;
}

// 设置充电协议
static int __maybe_unused set_charging_protocol(enum oplus_chg_protocol_type protocol)
{
    uint32_t protocol_to_be_switched;
    struct oplus_cpa *cpa;
    int rc = 0;
	

    if (!g_chg_ctrl.cpa_topic) {
        pr_err("CPA topic not available\n");
        rc = -ENODEV;
        goto out;
    }

	if ((protocol >= CHG_PROTOCOL_MAX) || (protocol <= CHG_PROTOCOL_INVALID)) {
		pr_err("unsupported protocol type, protocol=%d\n", protocol);
		rc = -EINVAL;
        goto out;
	}

	cpa = cs->oplus_mms_get_drvdata(g_chg_ctrl.cpa_topic);
    if (!cpa->retention_state) {
		if (!is_conecting) {
			pr_info("without connect, can't set protocol\n");
			rc = -EFAULT;
            goto out;
		}
	}

	pr_info("%s protocol identify request\n", get_protocol_name_str(protocol));

    // 请求指定的充电协议
	mutex_lock(&cpa->cpa_request_lock);

	protocol_to_be_switched = READ_ONCE(cpa->protocol_to_be_switched) | protocol;
	WRITE_ONCE(cpa->protocol_to_be_switched, protocol_to_be_switched);

	/* Suspend other requests before opening the request default protocol */
	if (!cpa->def_req) {
		pr_info("not request default protocol\n");
        goto out;
	}
	if (cpa->request_locked) {
		pr_info("request_locked\n");
        goto out;
	}

	schedule_work(&cpa->protocol_switch_work);

	mutex_unlock(&cpa->cpa_request_lock);

out:
    if (rc < 0) {
        pr_err("Failed to request protocol %s, rc=%d\n", get_protocol_name_str(protocol), rc);
        return rc;
    }

    pr_info("Requested charging protocol: %s\n", get_protocol_name_str(protocol));
    return rc;
}

// 应用电压和电流设置
static int apply_pps_voltage_current(int voltage_mv, int current_ma)
{
    struct oplus_pps *chip;
    int vol_set_mv = voltage_mv;
    int curr_set_ma = current_ma;
    int rc = 0;

    chip = cs->oplus_mms_get_drvdata(g_chg_ctrl.pps_topic);
    if (!chip) {
        pr_err("Failed to get PPS chip data\n");
        return -ENODEV;
    }

    if (!voltage_mv) {
        vol_set_mv = chip->vol_set_mv;
    }

    if (!current_ma) {
        curr_set_ma = chip->curr_set_ma;
    }
    
    // 调用PPS的PDO设置函数应用设置
    rc = cs->oplus_pps_pdo_set(chip, vol_set_mv, curr_set_ma);
    if (rc < 0) {
        pr_err("Failed to set PPS voltage %d mV and current %d mA, rc=%d\n", 
               vol_set_mv, curr_set_ma, rc);
        return rc;
    }
    
    pr_info("Successfully applied PPS voltage to %d mV and current to %d mA\n", 
            vol_set_mv, curr_set_ma);
    return 0;
}

static int apply_ufcs_voltage_current(int voltage_mv, int current_ma)
{
    struct oplus_ufcs *chip;
    int vol_set_mv = voltage_mv;
    int curr_set_ma = current_ma;
    int rc = 0;


    chip = cs->oplus_mms_get_drvdata(g_chg_ctrl.ufcs_topic);
    if (!chip) {
        pr_err("Failed to get UFCS chip data\n");
        return -ENODEV;
    }

    if (!voltage_mv) {
        vol_set_mv = chip->vol_set_mv;
    }

    if (!current_ma) {
        curr_set_ma = chip->curr_set_ma;
    }
    
    // 调用UFCS的PDO设置函数应用设置
    rc = cs->oplus_ufcs_pdo_set(chip, vol_set_mv, curr_set_ma);
    if (rc < 0) {
        pr_err("Failed to set UFCS voltage %d mV and current %d mA, rc=%d\n", 
               vol_set_mv, curr_set_ma, rc);
        return rc;
    }
    
    pr_info("Successfully applied UFCS voltage to %d mV and current to %d mA\n", 
            vol_set_mv, curr_set_ma);
    return 0;
}

// 设置电压电流和协议
int __maybe_unused set_protocol_voltage_current(int voltage_mv, int current_ma)
{
    int rc = 0;
    
    switch (current_protocol_type) {
    case CHG_PROTOCOL_PPS:
        rc = apply_pps_voltage_current(voltage_mv, current_ma);
        break;
        
    case CHG_PROTOCOL_UFCS:
        rc = apply_ufcs_voltage_current(voltage_mv, current_ma);
        break;
        
    case CHG_PROTOCOL_VOOC:
        pr_info("VOOC protocol does not support dynamic voltage/current setting\n");
        rc = -EINVAL;
        break;
        
    default:
        pr_err("Unsupported charging protocol: %d\n", current_protocol_type);
        rc = -EINVAL;
        break;
    }
    
    return rc;
}


// 启动充电
int __maybe_unused start_charging(void)
{
    int rc = 0;

    if (!g_chg_ctrl.chg_disable_votable) {
        pr_err("CHG_DISABLE votable not available\n");
        return -ENODEV;
    }

    rc = cs->vote(g_chg_ctrl.chg_disable_votable, CHG_CTRL_VOTER, false, 0, true);
    if (rc < 0) {
        pr_err("Failed to enable charging, rc=%d\n", rc);
        return rc;
    }

    pr_info("Charging started\n");
    return 0;
}

// 停止充电
int __maybe_unused stop_charging(void)
{
    int rc = 0;

    if (!g_chg_ctrl.chg_disable_votable) {
        pr_err("CHG_DISABLE votable not available\n");
        return -ENODEV;
    }

    rc = cs->vote(g_chg_ctrl.chg_disable_votable, CHG_CTRL_VOTER, true, 1, true);
    if (rc < 0) {
        pr_err("Failed to disable charging, rc=%d\n", rc);
        return rc;
    }

    pr_info("Charging stopped\n");
    return 0;
}

// 充电器插入事件处理回调函数
static void charging_plugin_handler(struct mms_subscribe *subs,
                                   enum mms_msg_type type, u32 id, bool sync)
{
    struct charging_control *ctrl = subs->priv_data;
    union mms_msg_data data = { 0 };

    switch (type) {
    case MSG_TYPE_ITEM:
        switch (id) {
        case WIRED_ITEM_ONLINE:
            // 获取充电器在线状态
            cs->oplus_mms_get_item_data(ctrl->wired_topic, id, &data, false);
            if (data.intval) {
                pr_info("Charger plugged in\n");
            } else {
                pr_info("Charger unplugged\n");
            }
            break;
        case WIRED_ITEM_PRESENT:
            // 获取充电器物理连接状态
            cs->oplus_mms_get_item_data(ctrl->wired_topic, id, &data, false);
            if (data.intval) {
                is_conecting = true;
                pr_info("Charger physically connected\n");
            } else {
                is_conecting = false;
                pr_info("Charger physically disconnected\n");
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

// 订阅wired topic以监听充电器插入事件
static int __maybe_unused subscribe_charger_plugin_event(struct charging_control *ctrl)
{
    if (!ctrl->wired_topic) {
        pr_err("Wired topic not available\n");
        return -ENODEV;
    }

    // 订阅wired topic的消息
    ctrl->wired_subs = cs->oplus_mms_subscribe(ctrl->wired_topic, ctrl,
                                          charging_plugin_handler, "charging_ctrl");
    if (IS_ERR_OR_NULL(ctrl->wired_subs)) {
        pr_err("Failed to subscribe to wired topic, rc=%ld\n",
               PTR_ERR(ctrl->wired_subs));
        return PTR_ERR(ctrl->wired_subs);
    }

    pr_info("Successfully subscribed to charger plugin events\n");
    return 0;
}

// 回调函数，当CPA状态发生变化时会被调用
static void cpa_protocol_callback(struct mms_subscribe *subs,
				 enum mms_msg_type type, u32 id, bool sync)
{
	struct charging_control *ctrl = subs->priv_data;
	union mms_msg_data data = { 0 };
	enum oplus_chg_protocol_type current_protocol;
	int rc;

	switch (type) {
	case MSG_TYPE_ITEM:
		switch (id) {
		case CPA_ITEM_CHG_TYPE:
			// 获取当前协议类型
			rc = cs->oplus_mms_get_item_data(ctrl->cpa_topic, CPA_ITEM_ALLOW, &data, false);
			if (rc < 0) {
				pr_err("Failed to get CPA protocol data, rc=%d\n", rc);
				return;
			}

			current_protocol = (enum oplus_chg_protocol_type)data.intval;
            current_protocol_type = current_protocol;
			break;
			
		default:
			pr_info("Other CPA item changed, id=%d\n", id);
			break;
		}
		break;
		
	default:
		pr_info("Received other message type: %d\n", type);
		break;
	}
}

// 等待并订阅CPA主题
static int subscribe_cpa_topic(struct charging_control *ctrl)
{
    if (!ctrl->cpa_topic) {
        pr_err("Wired topic not available\n");
        return -ENODEV;
    }

	// 订阅CPA主题
	ctrl->cpa_subs = cs->oplus_mms_subscribe(ctrl->cpa_topic, ctrl, cpa_protocol_callback, "cpa_listener");
    if (IS_ERR_OR_NULL(ctrl->cpa_subs)) {
        pr_err("Failed to subscribe to CPA topic, rc=%ld\n",
               PTR_ERR(ctrl->cpa_subs));
        return PTR_ERR(ctrl->cpa_subs);
    }
	
	pr_info("Successfully subscribed to CPA topic\n");
    return 0;
}

// 初始化充电控制模块
static int charging_control_init(void)
{
    g_chg_ctrl.gauge_topic = cs->oplus_mms_get_by_name("gauge");
    if (!g_chg_ctrl.gauge_topic) {
        pr_err("Failed to get gauge topic\n");
        return -ENODEV;
    }

    g_chg_ctrl.wired_topic = cs->oplus_mms_get_by_name("wired");
    if (!g_chg_ctrl.wired_topic) {
        pr_err("Failed to get wired topic\n");
        return -ENODEV;
    }

    g_chg_ctrl.cpa_topic = cs->oplus_mms_get_by_name("cpa");
    if (!g_chg_ctrl.cpa_topic) {
        pr_err("Failed to get cpa topic\n");
        return -ENODEV;
    }

    g_chg_ctrl.pps_topic = cs->oplus_mms_get_by_name("pps");
    if (!g_chg_ctrl.pps_topic) {
        pr_err("Failed to get pps topic\n");
        return -ENODEV;
    }

    g_chg_ctrl.ufcs_topic = cs->oplus_mms_get_by_name("ufcs");
    if (!g_chg_ctrl.ufcs_topic) {
        pr_err("Failed to get ufcs topic\n");
        return -ENODEV;
    }

    g_chg_ctrl.chg_disable_votable = cs->find_votable("WIRED_CHARGING_DISABLE");
    if (!g_chg_ctrl.chg_disable_votable) {
        pr_err("Failed to find charging disable votable\n");
        return -ENODEV;
    }

    g_chg_ctrl.wired_icl_votable = cs->find_votable("WIRED_ICL");
    if (!g_chg_ctrl.wired_icl_votable) {
        pr_err("Failed to find wired icl votable\n");
        return -ENODEV;
    }

    pr_info("chg_ctrl init success\n");
    return 0;
}

// 寻找需要的符号
static int find_symbols(void)
{
    int retries = 0;
    bool all_found = false;
    int i;

    while (retries < 3 && !all_found) {
        all_found = true;
        
        for (i = 0; i < chg_symbols_num; i++) {
            if (!chg_symbols_list[i].found) {
                unsigned long addr = lookup_symbol(chg_symbols_list[i].symbol_index);
                
                if (addr) {
                    *(chg_symbols_list[i].target) = (void *)addr;
                    chg_symbols_list[i].found = true;
                    pr_info("%d found at %px\n", chg_symbols_list[i].symbol_index, (void *)addr);
                } else {
                    all_found = false;
                    pr_info("Symbol %d not found, retrying...\n", chg_symbols_list[i].symbol_index);
                }
            }
        }
        
        if (!all_found) {
            msleep(100);
            retries++;
        }
    }

    if (!all_found) {
        pr_err("Failed to find all required symbols after %d retries\n", retries);
        return -ENOENT;
    }

    return 0;
}

static int cccv_thread(void *nothing)
{
	set_freezable();
	set_user_nice(current, 5);
	
	cccv_sleep_jiffies = msecs_to_jiffies(5000); /* 5 seconds */

	while (!kthread_should_stop()) {
        // 如果已经初始化完成，跳过初始化步骤
        if (!atomic_read(&all_init_done)) {
		    int rc;
		
		    rc = find_symbols();
		    if (rc != 0) {
			    schedule_timeout_interruptible(cccv_sleep_jiffies);
			    try_to_freeze();
			    continue;
		    }
		
		    rc = charging_control_init();
            if (rc != 0) {
			    pr_err("charging_control_init failed, rc=%d\n", rc);
			    schedule_timeout_interruptible(cccv_sleep_jiffies);
			    try_to_freeze();
			    continue;
		    }

            rc = subscribe_charger_plugin_event(&g_chg_ctrl);
            if (rc != 0) {
			    break;
		    }

            rc = subscribe_cpa_topic(&g_chg_ctrl);
		    if (rc == 0) {
			    atomic_set(&all_init_done, 1);
		    } else {
			    break;
		    }

            pr_info("all init done\n");
        }

        pr_info("voltage: %d current: %d\n", read_battery_voltage(), read_input_current());
        set_charging_protocol(CHG_PROTOCOL_PPS);
        //set_protocol_voltage_current(5000, 1000);

		schedule_timeout_interruptible(cccv_sleep_jiffies);
		try_to_freeze();
	}
	return 0;
}

static int __init cccv_module_init(void)
{
    struct task_struct *task;

    pr_info("Initializing cccv module\n");

    cs = kzalloc(sizeof(struct chg_symbols), GFP_KERNEL);
    if (!cs) {
        pr_err("Failed to allocate memory for chg symbols struct\n");
        return -ENOMEM;
    }

    init_chg_symbols_list();

    task = kthread_run(cccv_thread, NULL, "cccvd");
    if (IS_ERR(task)) {
        pr_err("Failed to initialize cccv module\n");
        return -EINVAL;
    }

    pr_info("cccv module initialized successfully\n");
    return 0;
}

static void __exit cccv_module_exit(void)
{
    pr_info("Exiting cccv module\n");

    // 清除投票
    if (g_chg_ctrl.chg_disable_votable)
        cs->vote(g_chg_ctrl.chg_disable_votable, "CHG_CTRL_VOTER", false, 0, false);
        
    if (g_chg_ctrl.wired_icl_votable)
        cs->vote(g_chg_ctrl.wired_icl_votable, "CHG_CTRL_VOTER", false, 0, false);

    kfree(cs);
    cs = NULL;
    atomic_set(&all_init_done, 0);
}

module_init(cccv_module_init);
module_exit(cccv_module_exit);

MODULE_DESCRIPTION("OPLUS cccv Module");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("brokestar233@github.com");
