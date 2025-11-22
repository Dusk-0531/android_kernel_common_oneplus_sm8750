// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2023 Oplus. All rights reserved.
 */

#ifndef __OPLUS_CHG_PPS_H__
#define __OPLUS_CHG_PPS_H__

#include <linux/miscdevice.h>

#include "oplus_mms.h"
#include "oplus_pps.h"
#include "oplus_chg_comm.h"

#define PPS_PDO_MAX			7
#define PPS_CMD_BUF_SIZE		128

#define PPS_PDO_VOL_MAX(pdo)		(pdo * 100)
#define PPS_PDO_VOL_MIN(pdo)		(pdo * 100)
#define PPS_PDO_CURR_MAX(pdo)		(pdo * 50)
#define PPS_STATUS_VOLT(pps_status) (((pps_status) >> 0) & 0xFFFF)
#define PPS_STATUS_CUR(pps_status) (((pps_status) >> 16) & 0xFF)

#define PD_PDO_VOL(pdo)			((pdo) * 50)
#define PD_PDO_CURR_MAX(pdo)		((pdo) * 10)

#define OPLUS_CHG_TRACK_SCENE_PPS_ERR	"pps_err"

enum pps_topic_item {
	PPS_ITEM_ONLINE,
	PPS_ITEM_CHARGING,
	PPS_ITEM_OPLUS_ADAPTER,
	PPS_ITEM_ONLINE_KEEP,
	PPS_ITEM_CPA_POWER,
	PPS_ITEM_ADAPTER_POWER,
	PPS_ITEM_PPS_ADAPTER_INFO,
	PPS_ITEM_ADAPTER_MAX_POWER,
};

typedef enum
{
	USBPD_PDMSG_PDOTYPE_FIXED_SUPPLY,
	USBPD_PDMSG_PDOTYPE_BATTERY,
	USBPD_PDMSG_PDOTYPE_VARIABLE_SUPPLY,
	USBPD_PDMSG_PDOTYPE_AUGMENTED
}USBPD_PDMSG_PDOTYPE_TYPE;

typedef union {
	u32 pdo_data;
	struct {
	        u32 max_current_10ma			: 10;    /*bit [ 9: 0]*/
	        u32 voltage_50mv			: 10;    /*bit [19:10]*/
	        u32 peak_current			: 2;    /*bit [21:20]*/
	        u32 					: 1;    /*bit [22:22]*/
	        u32 epr_mode_capable			: 1;    /*bit [23:23]*/
	        u32 unchunked_ext_msg_supported		: 1;    /*bit [24:24]*/
	        u32 dual_role_data			: 1;    /*bit [25:25]*/
	        u32 usb_comm_capable			: 1;    /*bit [26:26]*/
	        u32 unconstrained_pwer			: 1;    /*bit [27:27]*/
	        u32 usb_suspend_supported		: 1;    /*bit [28:28]*/
	        u32 dual_role_power			: 1;    /*bit [29:29]*/
	        u32 pdo_type				: 2;    /*bit [31:30]*/
	};
} pd_msg_data;

typedef union
{
	u32 pdo_data;
	struct {
		u32 max_current50ma           : 8;    /*bit [ 6: 0]*/
		u32 min_voltage100mv          : 8;    /*bit [15: 8]*/
		u32                           : 1;    /*bit [16:16]*/
		u32 max_voltage100mv          : 8;    /*bit [24:17]*/
		u32                           : 2;    /*bit [26:25]*/
		u32 pps_power_limited         : 1;    /*bit [27:27]*/
		u32 pps                       : 2;    /*bit [29:28]*/
		u32 pdo_type                  : 2;    /*bit [31:30]*/
	};
} pps_msg_data;

struct pps_dev_ops {
	int (*pps_pdo_set)(int vol_mv, int curr_ma);
	int (*verify_adapter)(void);
	int (*get_pdo_info)(u32 *pdo, int num);
	u32 (*get_pps_status)(void);
};

struct oplus_pps_phy_ic {
	struct pps_dev_ops *ops;
	int phy_ic_exist;
};

enum pps_fastchg_type {
	PPS_FASTCHG_TYPE_UNKOWN,
	PPS_FASTCHG_TYPE_THIRD,
	PPS_FASTCHG_TYPE_V1,
	PPS_FASTCHG_TYPE_V2,
	PPS_FASTCHG_TYPE_V3,
	PPS_FASTCHG_TYPE_OTHER,
};

enum pps_power_type {
	PPS_POWER_TYPE_UNKOWN = 0,
	PPS_POWER_TYPE_THIRD = 33,
	PPS_POWER_TYPE_V1 = 125,
	PPS_POWER_TYPE_V2 = 150,
	PPS_POWER_TYPE_V3 = 240,
	OPLUS_PPS_POWER_MAX = 0xFFFF,
};

enum pps_user_err_type {
	PPS_ERR_BTB_OVER = 1,
	PPS_ERR_TFG_OVER,
	PPS_ERR_IBAT_OVER,
	PPS_ERR_REQUEST_VOLT_OVER,
	PPS_ERR_MAX,
};

enum exit_pps_flag_status {
	EXIT_PPS_FALSE,
	EXIT_HIGH_PPS,
	EXIT_THIRD_PPS,
};

struct pps_dev_cmd {
	unsigned int cmd;
	unsigned int data_size;
	unsigned char data_buf[PPS_CMD_BUF_SIZE];
};

struct oplus_pps_config {
	unsigned int target_vbus_mv;
	unsigned int pps_target_curr_max_ma;
	int curr_max_ma;
	uint8_t *curve_strategy_name;
	int curr_max_ma_percent_75;
	int curr_max_ma_percent_85;
};

struct pps_full_curve {
	unsigned int iterm;
	unsigned int vterm;
	bool exit;
};

struct pps_full_curves_temp {
	struct pps_full_curve full_curves[FULL_PPS_SYS_MAX];
	int full_curve_num;
};

struct oplus_pps {
	struct device *dev;
	struct oplus_mms *err_topic;
	struct mms_subscribe *err_subs;
	struct oplus_mms *pps_topic;
	struct oplus_mms *cpa_topic;
	struct mms_subscribe *cpa_subs;
	struct oplus_mms *wired_topic;
	struct mms_subscribe *wired_subs;
	struct oplus_mms *comm_topic;
	struct mms_subscribe *comm_subs;
	struct oplus_mms *gauge_topic;
	struct mms_subscribe *gauge_subs;
	struct oplus_mms *batt_bal_topic;
	struct mms_subscribe *batt_bal_subs;
	struct oplus_mms *retention_topic;
	struct mms_subscribe *retention_subs;
	struct oplus_mms *plc_topic;
	struct mms_subscribe *plc_subs;
	struct oplus_chg_ic_dev *pps_ic;
	struct oplus_chg_ic_dev *cp_ic;
	struct oplus_chg_ic_dev *dpdm_switch;

	struct notifier_block nb;

	struct votable *pps_curr_votable;
	struct votable *pps_disable_votable;
	struct votable *pps_not_allow_votable;
	struct votable *wired_suspend_votable;
	struct votable *chg_disable_votable;
	struct votable *pps_boot_votable;
	struct votable *wired_icl_votable;

	struct delayed_work switch_check_work;
	struct delayed_work monitor_work;
	struct delayed_work current_work;
	struct delayed_work imp_uint_init_work;
	struct delayed_work boot_curr_limit_work;
	struct delayed_work switch_end_recheck_work;
	struct delayed_work retention_state_ready_work;

	struct work_struct wired_online_work;
	struct work_struct type_change_work;
	struct work_struct force_exit_work;
	struct work_struct soft_exit_work;
	struct work_struct gauge_update_work;
	struct work_struct retention_disconnect_work;
	struct work_struct close_cp_work;
	bool process_close_cp_item;
	struct work_struct cp_err_handler_work;
	struct work_struct cp_online_handler_work;
	struct work_struct cp_offline_handler_work;

	wait_queue_head_t read_wq;
	struct miscdevice misc_dev;
	struct mutex read_lock;
	struct mutex cmd_data_lock;
	struct completion cmd_ack;
	struct completion pd_svooc_wait_ack;
	struct pps_dev_cmd cmd;
	bool cmd_data_ok;

	struct oplus_chg_strategy *oplus_curve_strategy;
	struct oplus_chg_strategy *third_curve_strategy;
	struct oplus_chg_strategy *strategy;

	struct oplus_chg_strategy *oplus_lcf_strategy;
	struct oplus_chg_strategy *third_lcf_strategy;

	struct oplus_chg_strategy *temperature_strategy;
	uint8_t *temperature_strategy_name;
	uint8_t *temperature_strategy_data;
	uint32_t temperature_strategy_data_size;

	struct oplus_impedance_node *input_imp_node;
	struct oplus_impedance_unit *imp_uint;

	struct oplus_plc_protocol *opp;

	struct oplus_pps_config config;
	struct oplus_pps_timer timer;
	struct pps_protection_counts count;
	struct oplus_pps_limits limits;

	struct pps_full_curves_temp low_curr_full_curves_temp[PPS_LOW_CURR_FULL_CURVE_TEMP_MAX];

	u64 dev_info;
	u32 pps_status_info;
	pps_msg_data pdo[PPS_PDO_MAX];
	int pdo_num;
	bool pps_online;
	bool pps_online_keep;
	bool pps_charging;
	bool oplus_pps_adapter;
	bool pps_disable;
	bool pps_not_allow;
	enum oplus_cp_work_mode cp_work_mode;
	int last_target_curr_ma;
	int target_curr_ma;
	int target_vbus_mv;
	int curr_set_ma;
	int vol_set_mv;
	int bcc_max_curr;
	int bcc_min_curr;
	int bcc_exit_curr;
	int pps_connect_error_count;
	int pdo_set_error_count;
	enum exit_pps_flag_status retention_exit_pps_flag;
	enum oplus_chg_protocol_type cpa_current_type;
	int rmos_mohm;
	int cool_down;
	int error_count;
	int cp_ratio;

	bool wired_online;
	bool irq_plugin;
	bool disconnect_change;
	bool retention_state;
	bool retention_oplus_adapter;
	bool retention_state_ready;
	bool retention_ready_for_switch_end;
	bool led_on;
	bool need_check_current;
	bool mos_on_check;
	bool batt_hmac;
	bool batt_auth;
	int wired_type;
	bool support_cp_ibus;
	bool support_pps_status;
	int pps_curr_ma_from_pps_status;

	int ui_soc;
	int shell_temp;
	enum oplus_temp_region batt_temp_region;
	bool shell_temp_ready;
	int adapter_max_curr;
	int boot_time;
	int allow_check_soc;

	int pps_fastchg_batt_temp_status;
	int pps_temp_cur_range;
	int pps_low_curr_full_temp_status;
	bool quit_pps_protocol;
	int batt_bal_curr_limit;
	bool pdsvooc_id_adapter;
	bool request_vbus_too_low_flag;
	bool chg_ctrl_by_sale_mode;
	int delta_vbus[PPS_TEMP_RANGE_NORMAL + 1];
	bool enable_pps_status;
	bool lift_vbus_use_cpvout;
	int plc_status;
	unsigned int usb_status;

	struct dentry *debugfs_pps;
	int debug_force_pps_err;
	long pps_online_time;
	bool fcl_support;
	bool ss_check;
	bool fcl_trigger;
};


#endif /* __OPLUS_CHG_PPS_H__ */
