/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __CCCV_H
#define __CCCV_H
#include <linux/list.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#include <linux/sysms_finder.h>
#include <linux/changer/oplus_mms.h>
#include <linux/changer/oplus_chg_cpa.h>
#include <linux/changer/oplus_chg_pps.h>
#include <linux/changer/oplus_chg_comm.h>
#include <linux/changer/oplus_chg_ufcs.h>
#include <linux/changer/oplus_mms_wired.h>
#include <linux/changer/oplus_mms_gauge.h>



// 充电控制结构体
struct charging_control {
    struct oplus_mms *gauge_topic;
    struct oplus_mms *wired_topic;
    struct oplus_mms *cpa_topic;
    struct oplus_mms *pps_topic;
    struct oplus_mms *ufcs_topic;
    struct votable *chg_disable_votable;
    struct votable *wired_icl_votable;
    struct mms_subscribe *wired_subs;
    struct mms_subscribe *cpa_subs;
};

typedef struct oplus_mms * (*oplus_mms_get_by_name_t)(const char *name);
typedef int (*oplus_mms_get_item_data_t)(struct oplus_mms *mms, u32 item_id,
			    union mms_msg_data *data, bool update);
typedef int (*oplus_pps_pdo_set_t)(struct oplus_pps *chip, int vol_mv, int curr_ma);
typedef int (*oplus_ufcs_pdo_set_t)(struct oplus_ufcs *chip, int vol_mv, int curr_ma);
typedef int (*oplus_cpa_request_t)(struct oplus_mms *topic, enum oplus_chg_protocol_type type);
typedef int (*vote_t)(struct votable *votable, const char *client_str, bool state, int val, bool step);
typedef struct mms_subscribe * (*oplus_mms_subscribe_t)(
	struct oplus_mms *mms, void *priv_data,
	void (*callback)(struct mms_subscribe *, enum mms_msg_type, u32, bool),
	const char *format, ...);
typedef struct votable * (*find_votable_t)(const char *name);
typedef void * (*oplus_mms_get_drvdata_t)(struct oplus_mms *mms);

struct chg_symbols {
    oplus_mms_get_by_name_t             oplus_mms_get_by_name;
    oplus_mms_get_item_data_t         oplus_mms_get_item_data;
    oplus_pps_pdo_set_t                     oplus_pps_pdo_set;
    oplus_ufcs_pdo_set_t                   oplus_ufcs_pdo_set;
    oplus_cpa_request_t                     oplus_cpa_request;
    vote_t                                               vote;
    oplus_mms_subscribe_t                 oplus_mms_subscribe;
    find_votable_t                               find_votable;
    oplus_mms_get_drvdata_t             oplus_mms_get_drvdata;
};
extern struct chg_symbols *cs;

struct {
    int symbol_index;
    void **target;
    bool found;
} chg_symbols_list[] = {
    { SYMBOL_OPLUS_MMS_GET_BY_NAME,   NULL, false},
    { SYMBOL_OPLUS_MMS_GET_ITEM_DATA, NULL, false},
    { SYMBOL_OPLUS_PPS_PDO_SET,       NULL, false},
    { SYMBOL_OPLUS_UFCS_PDO_SET,      NULL, false},
    { SYMBOL_OPLUS_CPA_REQUEST,       NULL, false},
    { SYMBOL_VOTE,                    NULL, false},
    { SYMBOL_OPLUS_MMS_SUBSCRIBE,     NULL, false},
    { SYMBOL_FIND_VOTABLE,            NULL, false},
    { SYMBOL_OPLUS_MMS_GET_DRVDATA,   NULL, false},
};

static inline void init_chg_symbols_list(void)
{
    chg_symbols_list[0].target = (void **)&cs->oplus_mms_get_by_name;
    chg_symbols_list[1].target = (void **)&cs->oplus_mms_get_item_data;
    chg_symbols_list[2].target = (void **)&cs->oplus_pps_pdo_set;
    chg_symbols_list[3].target = (void **)&cs->oplus_ufcs_pdo_set;
    chg_symbols_list[4].target = (void **)&cs->oplus_cpa_request;
    chg_symbols_list[5].target = (void **)&cs->vote;
    chg_symbols_list[6].target = (void **)&cs->oplus_mms_subscribe;
    chg_symbols_list[7].target = (void **)&cs->find_votable;
    chg_symbols_list[8].target = (void **)&cs->oplus_mms_get_drvdata;
}

int chg_symbols_num = ARRAY_SIZE(chg_symbols_list);

#endif /* __CCCV_H */
