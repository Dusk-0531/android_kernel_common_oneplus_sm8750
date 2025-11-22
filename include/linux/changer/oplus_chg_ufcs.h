// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2023 Oplus. All rights reserved.
 */

#ifndef __OPLUS_CHG_UFCS_H__
#define __OPLUS_CHG_UFCS_H__

#include "oplus_mms.h"

#define UFCS_CMD_BUF_SIZE		128
#define FULL_UFCS_SYS_MAX		6
#define UFCS_VERIFY_AUTH_DATA_SIZE		16
#define UFCS_OUTPUT_MODE_MAX			7
#define UFCS_OPLUS_VND_POWER_INFO_MAX	7

enum ufcs_topic_item {
	UFCS_ITEM_ONLINE,
	UFCS_ITEM_CHARGING,
	UFCS_ITEM_ADAPTER_ID,
	UFCS_ITEM_OPLUS_ADAPTER,
	UFCS_ITEM_BCC_MAX_CURR,
	UFCS_ITEM_BCC_MIN_CURR,
	UFCS_ITEM_BCC_EXIT_CURR,
	UFCS_ITEM_BCC_TEMP_RANGE,
	UFCS_ITEM_SLOW_CHG_BATT_LIMIT,
	UFCS_ITEM_UFCS_VID,
	UFCS_ITEM_EMARK_POWER,
	UFCS_ITEM_ADAPTER_POWER,
	UFCS_ITEM_ERR_TYPE,
};

enum ufcs_fastchg_type {
	UFCS_FASTCHG_TYPE_UNKOWN,
	UFCS_FASTCHG_TYPE_THIRD = 0x8211,
	UFCS_FASTCHG_TYPE_V1 = 0x4211,
	UFCS_FASTCHG_TYPE_V2,
	UFCS_FASTCHG_TYPE_V3,
	UFCS_FASTCHG_TYPE_OTHER,
};

enum ufcs_power_type {
	UFCS_POWER_TYPE_UNKOWN = 18,
	UFCS_POWER_TYPE_V0 = 44,
	UFCS_POWER_TYPE_V1 = 45,
	UFCS_POWER_TYPE_V2 = 55,
	UFCS_POWER_TYPE_V3 = 67,
	UFCS_POWER_TYPE_V4 = 80,
	UFCS_POWER_TYPE_V5 = 88,
	UFCS_POWER_TYPE_V6 = 100,
	UFCS_POWER_TYPE_V7 = 120,
	UFCS_POWER_TYPE_V8 = 150,
	UFCS_POWER_TYPE_V9 = 240,
	UFCS_POWER_TYPE_MAX = 999,
};

enum ufcs_curr_table_type {
	UFCS_CURR_BIDIRECT_TABLE = 0,
	UFCS_CURR_CP_TABLE = 1,
};

enum ufcs_user_err_type {
	UFCS_ERR_IBUS_LIMIT = 1,
	UFCS_ERR_CP_ENABLE,
	UFCS_ERR_R_COOLDOWN,
	UFCS_ERR_BATT_BTB_COOLDOWN,
	UFCS_ERR_IBAT_OVER,
	UFCS_ERR_BTB_OVER,
	UFCS_ERR_MOS_OVER,
	UFCS_ERR_USBTEMP_OVER,
	UFCS_ERR_TFG_OVER,
	UFCS_ERR_VBAT_DIFF,
	UFCS_ERR_STARTUP_FAIL,
	UFCS_ERR_CIRCUIT_SWITCH,
	UFCS_ERR_ANTHEN_ERR,
	UFCS_ERR_PDO_ERR,
	UFCS_ERR_IMP,
	UFCS_ERR_MAX,
};

enum { UFCS_LOW_CURR_FULL_CURVE_TEMP_LITTLE_COOL,
       UFCS_LOW_CURR_FULL_CURVE_TEMP_NORMAL_LOW,
       UFCS_LOW_CURR_FULL_CURVE_TEMP_NORMAL_HIGH,
       UFCS_LOW_CURR_FULL_CURVE_TEMP_MAX,
};

enum exit_ufcs_flag_status {
	EXIT_UFCS_FALSE,
	EXIT_HIGH_UFCS,
	EXIT_THIRD_UFCS,
};

struct ufcs_dev_cmd {
	unsigned int cmd;
	unsigned int data_size;
	unsigned char data_buf[UFCS_CMD_BUF_SIZE];
};

struct ufcs_bcc_info {
	int bcc_min_curr;
	int bcc_max_curr;
	int bcc_exit_curr;
};

struct oplus_ufcs_config {
	unsigned int target_vbus_mv;
	int curr_max_ma;
	bool adsp_ufcs_project;
	bool ufcs_need_reset_adapter;
	int ufcs_boot_time_retry;
	uint8_t *curve_strategy_name;
	unsigned int high_imp_compensation_setting_mv;
	unsigned int upper_compensation_mv;
	int curr_max_ma_percent_75;
	int curr_max_ma_percent_85;
	int ufcs_watt_third;
};

struct oplus_ufcs_timer {
	struct timespec64 pdo_timer;
	struct timespec64 fastchg_timer;
	struct timespec64 temp_timer;
	unsigned long ibat_jiffies;
	int ufcs_max_time_ms;
	unsigned long monitor_jiffies;
};

struct ufcs_protection_counts {
	int cool_fw;
	int sw_full;
	int hw_full;
	int low_curr_full;
	int ibat_low;
	int ibat_high;
	int btb_high;
	int tbatt_over;
	int tfg_over;
	int output_low;
	int ibus_over;
};

struct oplus_ufcs_limits {
	int default_ufcs_normal_high_temp;
	int default_ufcs_little_cool_temp;
	int default_ufcs_little_cool_high_temp;
	int default_ufcs_cool_temp;
	int default_ufcs_little_cold_temp;
	int default_ufcs_normal_low_temp;
	int ufcs_warm_allow_vol;
	int ufcs_warm_allow_soc;

	int ufcs_batt_over_low_temp;
	int ufcs_little_cold_temp;
	int ufcs_cool_temp;
	int ufcs_little_cool_temp;
	int ufcs_little_cool_high_temp;
	int ufcs_normal_low_temp;
	int ufcs_normal_high_temp;
	int ufcs_batt_over_high_temp;
	int ufcs_strategy_temp_num;

	int ufcs_strategy_normal_current;
	int ufcs_strategy_batt_high_temp0;
	int ufcs_strategy_batt_high_temp1;
	int ufcs_strategy_batt_high_temp2;
	int ufcs_strategy_batt_low_temp2;
	int ufcs_strategy_batt_low_temp1;
	int ufcs_strategy_batt_low_temp0;
	int ufcs_strategy_high_current0;
	int ufcs_strategy_high_current1;
	int ufcs_strategy_high_current2;
	int ufcs_strategy_low_current2;
	int ufcs_strategy_low_current1;
	int ufcs_strategy_low_current0;

	int ufcs_low_curr_full_cool_temp;
	int ufcs_low_curr_full_little_cool_temp;
	int ufcs_low_curr_full_normal_low_temp;
	int ufcs_low_curr_full_normal_high_temp;

	int ufcs_over_high_or_low_current;
	int ufcs_strategy_change_count;
	int ufcs_full_cool_sw_vbat;
	int ufcs_full_normal_sw_vbat;
	int ufcs_full_normal_hw_vbat;
	int ufcs_full_warm_vbat;
	int ufcs_full_cool_sw_vbat_third;
	int ufcs_full_normal_sw_vbat_third;
	int ufcs_full_normal_hw_vbat_third;
	int ufcs_timeout_third;
	int ufcs_timeout_oplus;
	int ufcs_ibat_over_third;
	int ufcs_ibat_over_oplus;
	int ufcs_ibus_over_oplus;

	int ufcs_low_temp;
	int ufcs_high_temp;
	int ufcs_low_soc;
	int ufcs_high_soc;
	int ufcs_high_soc_third;
	int ufcs_removed_bat_decidegc;
};

struct ufcs_full_curve {
	unsigned int iterm;
	unsigned int vterm;
	bool exit;
};

struct ufcs_full_curves_temp {
	struct ufcs_full_curve full_curves[FULL_UFCS_SYS_MAX];
	int full_curve_num;
};

struct oplus_ufcs {
	struct device *dev;
	struct oplus_mms *ufcs_topic;
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
	struct oplus_mms *err_topic;
	struct oplus_mms *retention_topic;
	struct mms_subscribe *retention_subs;
	struct oplus_mms *plc_topic;
	struct mms_subscribe *plc_subs;
	struct oplus_chg_ic_dev *ufcs_ic;
	struct oplus_chg_ic_dev *cp_ic;
	struct oplus_chg_ic_dev *dpdm_switch;

	struct notifier_block nb;

	struct votable *ufcs_curr_votable;
	struct votable *ufcs_disable_votable;
	struct votable *ufcs_not_allow_votable;
	struct votable *ufcs_boot_votable;
	struct votable *wired_suspend_votable;
	struct votable *wired_icl_votable;
	struct votable *chg_disable_votable;

	struct delayed_work switch_check_work;
	struct delayed_work monitor_work;
	struct delayed_work current_work;
	struct delayed_work imp_uint_init_work;
	struct delayed_work wait_auth_data_work;
	struct delayed_work ufcs_restart_timeout_work;
	struct delayed_work ufcs_eis_work;
	struct delayed_work send_authdata_to_adsp_work;
	struct delayed_work switch_end_recheck_work;

	struct work_struct wired_online_work;
	struct work_struct force_exit_work;
	struct work_struct soft_exit_work;
	struct work_struct gauge_update_work;
	struct work_struct err_flag_push_work;
	struct work_struct fifo_overflow_push_work;
	struct work_struct plat_notify_exit_work;
	struct work_struct ufcs_restart_work;
	struct work_struct retention_disconnect_work;
	struct work_struct handshake_error_work;
	struct work_struct fgreset_ufcs_retry_work;
	struct work_struct cp_err_handler_work;
	struct work_struct cp_online_handler_work;
	struct work_struct cp_offline_handler_work;

	wait_queue_head_t read_wq;
	struct miscdevice misc_dev;
	struct mutex read_lock;
	struct mutex cmd_data_lock;
	struct completion cmd_ack;
	struct ufcs_dev_cmd cmd;
	bool cmd_data_ok;
	char auth_data[UFCS_VERIFY_AUTH_DATA_SIZE];
	bool auth_data_ok;
	bool wait_auth_data_done;

	struct oplus_chg_strategy *oplus_curve_strategy;
	struct oplus_chg_strategy *third_curve_strategy;
	struct oplus_chg_strategy *strategy;

	struct oplus_chg_strategy **oplus_lcf_strategy;
	struct oplus_chg_strategy **third_lcf_strategy;
	int oplus_lcf_num;
	int third_lcf_num;

	struct oplus_chg_strategy *temperature_strategy;
	uint8_t *temperature_strategy_name;
	uint8_t *temperature_strategy_data;
	uint32_t temperature_strategy_data_size;

	struct oplus_impedance_node *input_imp_node;
	struct oplus_impedance_unit *imp_uint;

	struct oplus_plc_protocol *opp;

#if IS_ENABLED(CONFIG_OPLUS_DYNAMIC_CONFIG_CHARGER)
	struct oplus_cfg debug_cfg;
#endif

	struct oplus_ufcs_config config;
	struct oplus_ufcs_timer timer;
	struct ufcs_protection_counts count;
	struct oplus_ufcs_limits limits;
	struct ufcs_bcc_info bcc;

	struct ufcs_full_curves_temp low_curr_full_curves_temp[UFCS_LOW_CURR_FULL_CURVE_TEMP_MAX];

	u64 dev_info;
	u64 src_info;
	u64 cable_info;
	u64 emark_info;
	u64 pdo[UFCS_OUTPUT_MODE_MAX];
	int pdo_num;
	u64 pie[UFCS_OPLUS_VND_POWER_INFO_MAX];
	int pie_num;
	unsigned int err_flag;
	bool ufcs_online;
	bool ufcs_charging;
	bool handshake_ok;
	bool adapter_check_third_ufcs;
	int retention_ufcs_power;
	bool oplus_ufcs_adapter;
	bool ufcs_disable;
	bool ufcs_not_allow;
	enum oplus_cp_work_mode cp_work_mode;
	bool oplus_cp_ucp_disable;
	int emark_imax;
	int power_imax;
	int allow_check_soc;

	int last_target_curr_ma;
	int target_curr_ma;
	int target_vbus_mv;
	int curr_set_ma;
	int curr_cc;
	int vol_set_mv;
	int bcc_max_curr;
	int bcc_min_curr;
	int bcc_exit_curr;
	int rmos_mohm;
	int cool_down;
	int curr_table_type;
	u32 adapter_id;
	int start_retry_count;
	bool start_check;
	int cp_ratio;

	bool wired_online;
	bool batt_hmac;
	bool batt_auth;
	bool irq_plugin;
	bool disconnect_change;
	bool retention_state;
	bool retention_oplus_adapter;
	bool retention_state_ready;
	int connect_error_count;
	int connect_error_count_level;
	int ufcs_handshake_error_count;
	bool adapter_verify_fail_flag;
	enum exit_ufcs_flag_status retention_exit_ufcs_flag;
	enum oplus_chg_protocol_type cpa_current_type;

	int high_soc;
	int ui_soc;
	int shell_temp;
	enum oplus_temp_region batt_temp_region;
	bool shell_temp_ready;
	int adapter_max_curr;

	int ufcs_fastchg_batt_temp_status;
	int ufcs_temp_cur_range;
	int ufcs_low_curr_full_temp_status;
	int batt_bal_curr_limit;
	int preliminary_imp_check_cnt;
	bool need_preliminary_imp_check;

	bool reset_adapter;
	struct completion reset_abnormal_ack;
	bool slow_chg_enable;
	int slow_chg_pct;
	int slow_chg_watt;
	int slow_chg_batt_limit;
	int startup_retry_times;
	bool chg_ctrl_by_sale_mode;
	int subboard_ntc_abnormal_current;
	bool prot_crash;
	int eis_status;
	bool lift_vbus_use_cpvout;
	bool ufcs_dispatch_key_ok;
	int plc_status;
	int plc_curr;
	u16 ufcs_vid;
	int batt_alarm;
	unsigned int usb_status;
	int ufcs_err_type;
	bool fcl_support;
	bool ss_check;
	bool fcl_trigger;
};

#endif /* __OPLUS_CHG_UFCS_H__ */
