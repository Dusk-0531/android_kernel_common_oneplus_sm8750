// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021-2021 Oplus. All rights reserved.
 */

#ifndef __OPLUS_MMS_H__
#define __OPLUS_MMS_H__

#include <linux/spinlock.h>

#define TOPIC_NAME_MAX		128
#define TOPIC_MSG_STR_BUF	1024

#define TOPIC_SUBS_RETRY_DELAY	100
#define TOPIC_SUBS_RETRY_MAX	50000

enum oplus_mms_type {
	OPLUS_MMS_TYPE_UNKNOWN,
	OPLUS_MMS_TYPE_ERROR,
	OPLUS_MMS_TYPE_GAUGE,
	OPLUS_MMS_TYPE_BATTERY,
	OPLUS_MMS_TYPE_USB,
	OPLUS_MMS_TYPE_WLS,
	OPLUS_MMS_TYPE_TEMP,
	OPLUS_MMS_TYPE_VOOC,
	OPLUS_MMS_TYPE_AIRVOOC,
	OPLUS_MMS_TYPE_COMM,
	OPLUS_MMS_TYPE_PARALLEL,
	OPLUS_MMS_TYPE_DUAL_CHAN,
	OPLUS_MMS_TYPE_CPA,
	OPLUS_MMS_TYPE_UFCS,
	OPLUS_MMS_TYPE_PPS,
	OPLUS_MMS_TYPE_BATT_BAL,
	OPLUS_MMS_TYPE_LEVEL_SHIFT,
	OPLUS_MMS_TYPE_RETENTION,
	OPLUS_MMS_TYPE_PLC,
};

enum mms_msg_type {
	MSG_TYPE_TIMER,
	MSG_TYPE_ITEM,
};

enum mms_msg_prio {
	MSG_PRIO_HIGH,
	MSG_PRIO_MEDIUM,
	MSG_PRIO_LOW,
};

struct oplus_mms;

union mms_msg_data {
	int intval;
	char *strval;
};

struct mms_item_desc {
	u32 item_id;
	bool str_data;
	bool up_thr_enable;
	bool down_thr_enable;
	bool dead_thr_enable;
	int update_thr_up;
	int update_thr_down;
	int dead_zone_thr;
	int (*update)(struct oplus_mms *, union mms_msg_data *);
};

struct mms_item {
	struct mms_item_desc desc;
	bool updated;
	rwlock_t lock;
	bool disabled;
	struct mutex update_lock;
	union mms_msg_data data;
	union mms_msg_data pre_data;
};

enum mms_msg_payload {
	MSG_LOAD_NULL,
	MSG_LOAD_INT,
	MSG_LOAD_STR,
};

struct mms_msg {
	enum mms_msg_type type;
	enum mms_msg_prio prio;
	u32 item_id;
	struct list_head list;
	enum mms_msg_payload payload;
	bool sync;
	u8 buf[];
};

struct mms_subscribe {
	char name[TOPIC_NAME_MAX];
	struct oplus_mms *mms;
	void *priv_data;
	struct list_head list;
	struct list_head callback_list;
	struct list_head callback_list_sync;
	void (*callback)(struct mms_subscribe *, enum mms_msg_type, u32, bool sync);
};

struct oplus_mms_config {
	struct device_node *of_node;
	struct fwnode_handle *fwnode;

	/* Driver private data */
	void *drv_data;

	int update_interval;
	/* Device specific sysfs attributes */
	const struct attribute_group **attr_grp;
};

struct oplus_mms_desc {
	const char *name;
	enum oplus_mms_type type;
	struct mms_item *item_table;
	int item_num;
	const u32 *update_items;
	int update_items_num;
	int update_interval;
	void (*update)(struct oplus_mms *, bool publish);
	void (*set_update_mode)(struct oplus_mms *, bool mode);
};

struct oplus_mms {
	const struct oplus_mms_desc *desc;
	int static_update_interval;
	int normal_update_interval;
	struct list_head subscribe_list;
	struct list_head msg_list;
	spinlock_t subscribe_lock;
	struct mutex msg_lock;
	struct mutex sync_msg_lock;
	struct delayed_work update_work;
	struct delayed_work msg_work;
	struct work_struct callback_work;

	struct device_node *of_node;
	void *drv_data;
	struct device dev;
	spinlock_t changed_lock;
	bool changed;
	bool initialized;
	bool removing;
	atomic_t use_cnt;
	bool force_update;

#ifdef CONFIG_OPLUS_CHG_MMS_DEBUG
	u32 debug_item_id;
	struct mms_subscribe *debug_subs;
#endif /* CONFIG_OPLUS_CHG_MMS_DEBUG */
};

typedef void (*mms_callback_t)(struct oplus_mms *topic, void *data);

#define to_oplus_mms(device) container_of(device, struct oplus_mms, dev)

static inline int oplus_mms_update_interval(struct oplus_mms *mms)
{
	if (!mms)
		return 0;
	return mms->normal_update_interval;
}

#endif /* __OPLUS_MMS_H__ */
