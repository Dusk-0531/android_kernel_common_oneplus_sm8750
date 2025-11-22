// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2023 Oplus. All rights reserved.
 */

#ifndef __OPLUS_CHG_CPA_H__
#define __OPLUS_CHG_CPA_H__

#include <linux/spinlock.h>

enum oplus_chg_protocol_type {
	CHG_PROTOCOL_INVALID = -1,
	CHG_PROTOCOL_BC12 = 0,
	CHG_PROTOCOL_PD,
	CHG_PROTOCOL_PPS,
	CHG_PROTOCOL_VOOC,
	CHG_PROTOCOL_UFCS,
	CHG_PROTOCOL_QC,
	CHG_PROTOCOL_MAX,
};

enum cpa_topic_item {
	CPA_ITEM_CHG_TYPE,
	CPA_ITEM_ALLOW,
	CPA_ITEM_TIMEOUT,
	CPA_ITEM_POWER,
};

struct protocol_map {
	uint32_t protocol;
	int type[CHG_PROTOCOL_MAX];
};

struct oplus_cpa_protocol_info {
	enum oplus_chg_protocol_type type;
	int power_mw;
	int max_power_mw;
};

struct oplus_cpa_protocol_wait_info {
	enum oplus_chg_protocol_type type;
	int time; /* ms */
};

struct oplus_cpa {
	struct device *dev;
	struct oplus_mms *cpa_topic;
	struct oplus_mms *wired_topic;
	struct oplus_mms *vooc_topic;
	struct mms_subscribe *wired_subs;
	struct mms_subscribe *vooc_subs;
	struct oplus_mms *ufcs_topic;
	struct oplus_mms *pps_topic;
	struct mms_subscribe *ufcs_subs;
	struct mms_subscribe *pps_subs;
	struct oplus_mms *retention_topic;
	struct mms_subscribe *retention_subs;

	struct votable *req_lock_votable;
	struct work_struct protocol_switch_work;
	struct work_struct chg_type_change_work;
	struct work_struct fast_chg_type_change_work;
	struct work_struct wired_offline_work;
	struct work_struct wired_online_work;
	struct work_struct switch_end_work;
	struct delayed_work protocol_switch_timeout_work;
	struct delayed_work protocol_ready_timeout_work;

#if IS_ENABLED(CONFIG_OPLUS_DYNAMIC_CONFIG_CHARGER)
	struct oplus_cfg debug_cfg;
#endif

	enum oplus_chg_protocol_type current_protocol_type;
	uint32_t protocol_to_be_switched;
	unsigned long protocol_disable_mask;
	uint32_t default_protocol_type;
	unsigned long ready_protocol_type;
	uint32_t protocol_supported_type;
	struct oplus_cpa_protocol_info protocol_prio_table[CHG_PROTOCOL_MAX];
	bool def_req;
	bool request_pending;
	bool started;
	bool request_locked;
	bool status_reset;

	bool wired_online;
	int cc_detect;
	int wired_type;
	int wired_real_chg_type;

	bool vooc_started;
	bool vooc_online;
	bool vooc_charging;
	bool vooc_online_keep;
	unsigned int vooc_sid;

	bool ufcs_online;
	bool ufcs_charging;
	bool ufcs_oplus_adapter;
	u32 ufcs_adapter_id;

	bool pps_online;
	bool pps_online_keep;
	bool pps_charging;
	bool pps_oplus_adapter;

	bool retention_state;
	bool pre_retention_state;
	bool retention_state_ready;

	bool wired_present;
	unsigned long wired_plugout_time;

	struct mutex cpa_request_lock;
	struct mutex start_lock;

	uint8_t region_id;

	bool pd_comleted;
	bool protocol_wait_support;
	int protocol_wait_cnt;
	struct completion pd_completed_ack;
	struct oplus_cpa_protocol_wait_info protocol_wait_table[CHG_PROTOCOL_MAX];
};

const char * const protocol_name_str[] = {
	[CHG_PROTOCOL_BC12]	= "BC1.2",
	[CHG_PROTOCOL_PD]	= "PD",
	[CHG_PROTOCOL_PPS]	= "PPS",
	[CHG_PROTOCOL_VOOC]	= "VOOC",
	[CHG_PROTOCOL_UFCS]	= "UFCS",
	[CHG_PROTOCOL_QC]	= "QC",
};

const char *get_protocol_name_str(enum oplus_chg_protocol_type type)
{
	if (type < 0 || type >= CHG_PROTOCOL_MAX)
		return "Unknown";
	return protocol_name_str[type];
}

#endif /* __OPLUS_CHG_CPA_H__ */
