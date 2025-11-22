#ifndef __OPLUS_MMS_GAUGE_H__
#define __OPLUS_MMS_GAUGE_H__

#include "oplus_mms.h"
#include "oplus_chg.h"

#define GAUGE_INVALID_TEMP	(-400)
#define OPLUS_BATTERY_TYPE_LEN 16

enum gauge_topic_item {
	GAUGE_ITEM_SOC,
	GAUGE_ITEM_VOL,
	GAUGE_ITEM_VOL_MAX,
	GAUGE_ITEM_VOL_MIN,
	GAUGE_ITEM_GAUGE_VBAT,
	GAUGE_ITEM_CURR,
	GAUGE_ITEM_TEMP,
	GAUGE_ITEM_FCC,
	GAUGE_ITEM_CC,
	GAUGE_ITEM_SOH,
	GAUGE_ITEM_RM,
	GAUGE_ITEM_BATT_EXIST,
	GAUGE_ITEM_ERR_CODE,
	GAUGE_ITEM_RESUME,
	GAUGE_ITEM_HMAC,
	GAUGE_ITEM_AUTH,
	GAUGE_ITEM_REAL_TEMP,
	GAUGE_ITEM_SUBBOARD_TEMP_ERR,
	GAUGE_ITEM_VBAT_UV,
	GAUGE_ITEM_DEEP_SUPPORT,
	GAUGE_ITEM_REG_INFO,
	GAUGE_ITEM_CALIB_TIME,
	GAUGE_ITEM_UV_INC,
	GAUGE_ITEM_FCC_COEFF,
	GAUGE_ITEM_SOH_COEFF,
	GAUGE_ITEM_SILI_IC_ALG_DSG_ENABLE,
	GAUGE_ITEM_SILI_IC_ALG_CFG,
	GAUGE_ITEM_SPARE_POWER_ENABLE,
	GAUGE_ITEM_SILI_IC_ALG_TERM_VOLT,
	GAUGE_ITEM_LIFETIME_STATUS,
	GAUGE_ITEM_RATIO_VALUE,
	GAUGE_ITEM_RATIO_TRANGE,
	GAUGE_ITEM_QMAX,
	GAUGE_ITEM_CAR_C,
	GAUGE_ITEM_RATIO_LIMIT_CURR,
	GAUGE_ITEM_SUB_BTB_STATE,
	GAUGE_ITEM_GAUGE_R_INFO,
	GAUGE_ITEM_SOC_CENTI,
	GAUGE_ITEM_VOL_FCL,
};

enum gauge_type_id {
	DEVICE_BQ27541,
	DEVICE_BQ27411,
	DEVICE_BQ28Z610,
	DEVICE_ZY0602,
	DEVICE_ZY0603,
	DEVICE_NFG8011B,
	DEVICE_SN28Z729 = 7,
	DEVICE_MPC7022,
};

typedef enum {
	DEC_CV_PACK_UNKNOWN,
	DEC_CV_PACK_DOUBLE_SERIES,
	DEC_CV_PACK_DOUBLE_PARALLE,
	DEC_CV_PACK_SINGLE,
	DEC_CV_PACK_SOH,
	DEC_CV_QCOM_FG,
	DEC_CV_MTK_FG,
	DEC_CV_MB_TI,
	DEC_CV_MB_CW,
	DEC_CV_MB_SMI,
	DEC_CV_PACK_MAX,
} DEC_CV_PACK_TYPE;

#define OPLUS_BATTINFO_DATE_SIZE 11
#define OPLUS_BATT_SERIAL_NUM_SIZE 20
#define CHEM_ID_LENGTH 8
#define CHEMID_MAX_LENGTH 512

enum batt_connect_type {
	DEFAULT_CONNECT_TYPE,
	PARALLEL_CONNECT_TYPE,
	SERIAL_CONNECT_TYPE,
};

enum {
    GAUGE_TYPE_UNKNOW = 0,
    GAUGE_TYPE_PLATFORM = 1,
    GAUGE_TYPE_PACK = 2,
    GAUGE_TYPE_BOARD = 3,
    GAUGE_TYPE_MAX,
};

#define BATT_SUB_BTB_ABNORMAL_MAX_CURR	9500
#define BATT_SUB_BTB_ABNORMAL_MIN_CURR	2000

struct battery_manufacture_info {
    u16 manu_date;
    u16 first_usage_date;
    u16 ui_cycle_count;
    u8 ui_soh;
    u8 used_flag;
    char batt_serial_num[OPLUS_BATT_SERIAL_NUM_SIZE];
} __attribute__((packed));

#define GAUGE_CALIB_ARGS_LEN 12
struct gauge_calib_info {
	int dod_time;
	int qmax_time;
	unsigned char calib_args[GAUGE_CALIB_ARGS_LEN];
}__attribute__((aligned(4)));

#define OPLUS_GAUGE_THREE_LEVEL_TERM_VOLT_LEN   18
#define OPLUS_GAUGE_CUV_STATE_CHECK_TRY_MAX    2
#define OPLUS_GAUGE_CUV_STATE_1    1
#define OPLUS_GAUGE_CUV_STATE_2    0

enum battery_test_state {
	BATTERY_STOP_TEST_TYPE,
	BATTERY_START_TEST_TYPE,
	BATTERY_MAX_TEST_TYPE,
};

struct oplus_gauge_nvram_stress_test {
	int input_count;
	int interval_ms;
	int input_state;

	/* nvram test */
	int sum_cnt;
	int fail_cnt;
	int suc_cnt;

	/* term volt test */
	int term_volt_sum_cnt;
	int term_volt_fail_cnt;
	int term_volt_suc_cnt;

	/* normal gauge test */
	int read_sum_cnt;
	int read_fail_cnt;
	int read_suc_cnt;

	int test_state;
	int read_test_state;
	int term_volt_test_state;
};

#endif /* __OPLUS_MMS_GAUGE_H__ */
