#pragma once
#include "sns_demo_common.h"

#include "sns_demo_ver.h"
#include "sns_demo_hal.h"
#include "sns_demo_sensor_instance.h"


#define SENSOR_NAME "demo"
#define VENDOR_NAME "BOSCH"


#ifndef SUID_IS_NULL
#define SUID_IS_NULL(suid_ptr) \
    (sns_memcmp((suid_ptr), &(sns_sensor_uid){{0}}, sizeof(sns_sensor_uid)) == 0)
    // (sns_sensor_uid_compare((suid_ptr), NULL) == true)
#endif

#define ACCEL_SUID_0 { \
    .sensor_uid = { \
        0x4A,0xFF,0x70,0x8E,0xE7,0xC7,0x45,0xC7,\
        0x29,0xA3,0x2A,0x8A,0x83,0x70,0xF1,0xA6 \
    } \
}

#define ACCEL_SUID_1 { \
    .sensor_uid = { \
        0x80,0x22,0x58,0x7A,0xE2,0xAB,0xFE,0xAC,\
        0xD2,0x7A,0xE3,0x84,0x95,0xC6,0x7C,0x21 \
    } \
}

#define GYRO_SUID_0 { \
    .sensor_uid = { \
        0x4F,0xD9,0x70,0x73,0x99,0x2C,0xB1,0x0B,\
        0x85,0x92,0x54,0xF2,0xE8,0xE4,0xA4,0xEC \
    } \
}

#define GYRO_SUID_1 { \
    .sensor_uid = { \
        0x6E,0x3E,0xB6,0x66,0x4D,0x8F,0xD5,0x06,\
        0x33,0x52,0xCC,0xF5,0x97,0xCB,0x0B,0xF5 \
    } \
}

#define MD_SUID_0 { \
    .sensor_uid = { \
        0x0C,0x0B,0x9C,0x07,0xB5,0xA9,0x11,0x7A,\
        0xF2,0x3D,0xA8,0x7D,0x39,0x72,0x5C,0x42 \
    } \
}

#define MD_SUID_1 { \
    .sensor_uid = { \
        0x2C,0x6E,0x2A,0x5C,0x77,0x40,0x46,0x9E,\
        0xB2,0x30,0x78,0x7E,0x76,0x55,0x86,0x41 \
    } \
}

#define TEMP_SUID_0 { \
    .sensor_uid = { \
        0x79,0x0A,0x08,0xAF,0x62,0xAD,0x92,0xAA,\
        0x7A,0x05,0xBC,0xE8,0x8C,0x7E,0x02,0xEE \
    } \
}

#define TEMP_SUID_1 { \
    .sensor_uid = { \
        0x1E,0x70,0x67,0x2B,0x30,0xEF,0x56,0xDD,\
        0x82,0x1C,0xC9,0xB1,0xFC,0x1B,0x4E,0xB7 \
    } \
}

#define DEMO_SCALE_FACTOR_DATA_ACCEL     (1e7)  //can be 1e6 if we use +/- 8g or +/-16g range is used
#define DEMO_SCALE_FACTOR_DATA_DFT       (1e6)
#define DEMO_SCALE_FACTOR_DATA_GYRO      (DEMO_SCALE_FACTOR_DATA_DFT)
#define DEMO_SCALE_FACTOR_DATA_TEMP      (DEMO_SCALE_FACTOR_DATA_DFT)

/* maybe better to move it to common.h */
typedef enum {
    DEMO_POWER_RAIL_PENDING_NONE,
    DEMO_POWER_RAIL_PENDING_INIT,
    DEMO_POWER_RAIL_PENDING_SET_CLIENT_REQ,
    DEMO_POWER_RAIL_PENDING_DEINIT,
} demo_power_rail_pending_state;






struct demo_common_state {
    uint8_t chip_id;
    uint8_t i2c_dummy;
    uint8_t i3c_dummy;
    uint8_t spi_dummy;

    SNS_SUID_LOOKUP_DATA(5) suid_lookup_data;

    /* hardware related to be read from registry files */
    // sns_registry_phy_sensor_cfg registry_cfg;
    // sns_registry_phy_sensor_pf_cfg registry_pf_cfg;
    demo_state *states[NUMS];
    demo_state *acc_state;
    demo_state *gyr_state;
    demo_state *temp_state;
    demo_state *md_state;
    // ss_cfg_t acc_ss_cfg;
    // ss_cfg_t gyr_ss_cfg;
    // ss_cfg_t tmp_ss_cfg;
    // ss_cfg_t md_ss_cfg;
    pf_cfg_t pf_cfg;
    md_cfg_t md_cfg;
    bool registry_pf_cfg_received;
    bool registry_md_cfg_received;
    bool registry_placement_received;
    bool registry_orient_received;
    uint8_t registry_req_cnt;

    irq_info_t irq_info;
    com_port_info com_port;
    sns_rail_config rail_cfg;
    sns_power_rail_state registry_rail_on_state;

    float placement[12];
    triaxis_conversion axis_map[3];
};

struct demo_state {
    uint8_t hw_idx;
    ss_cfg_t ss_cfg;
    sns_sensor_uid my_suid;
    demo_sensor_idx  sensor_idx;
    demo_sensor_type sensor_type;
    /* data */
    demo_common_state *common;
    sns_sensor_uid irq_suid;
    sns_sensor_uid ascp_suid;
    sns_sensor_uid timer_suid;
    sns_sensor_uid registry_suid;

    /* data stream */
    sns_data_stream *suid_stream;
    sns_data_stream *timer_stream;
    sns_data_stream *registry_stream;

    /* service */
    sns_diag_service *diag_service;
    sns_sync_com_port_service *scp_service;
    sns_pwr_rail_service *pwr_rail_service;


    bool hw_is_present;
    bool hw_detection_finished;
    bool registry_ss_cfg_received;
    demo_power_rail_pending_state pwrail_pending_state;


    bool registry_fac_cal_received;
    matrix3 fac_cal_corr_mat;
    int32_t fac_cal_bias[3];
    float fac_cal_scale[3];
    uint32_t fac_cal_version;
    //uint32_t scale_factor;
    float scale_factor;

    // float resolutions[4];
    float resolutions[5];

    size_t encoded_event_len;

    float chosen_sample_rate;

    SNS_SUID_LOOKUP_DATA(5) suid_lookup_data;

};






void demo_common_init(sns_sensor *const this);
void demo_sensor_send_registry_request(sns_sensor *const this);
void demo_publish_default_registry_attributes(sns_sensor *const this);
void demo_update_registry_attributes(sns_sensor *const this);
void demo_sensor_check_registry_finished(sns_sensor *const this);
void demo_sensor_parse_registry(sns_sensor *const this, sns_sensor_event *event);

void demo_sensor_update_power_rail_vote(sns_sensor *const this, sns_power_rail_state vote,
                                  sns_time *on_timestamp);
void demo_sensor_start_power_rail_timer(sns_sensor *const this, sns_time timeout_ticks,
                                        demo_power_rail_pending_state pwrail_pend_state);

void demo_sensor_exit_island(sns_sensor *const this);

sns_rc demo_start_hw_detect(sns_sensor *const this);
void demo_sensor_register_pf_resource(sns_sensor *const this);

void demo_publish_available(sns_sensor *const this);

sns_sensor *demo_get_master_sensor(sns_sensor *this, demo_sensor_type sensor_type);

void demo_reval_instance_config(sns_sensor *const this, sns_sensor_instance *instance);
void demo_set_client_config(sns_sensor *const this, sns_sensor_instance *instance,
                            demo_req_payload req_payload, uint32_t message_id);



sns_rc demo_acc_init(sns_sensor *const this);
sns_rc demo_acc_deinit(sns_sensor *const this);
sns_rc demo_gyro_init(sns_sensor *const this);
sns_rc demo_gyro_deinit(sns_sensor *const this);
sns_rc demo_md_init(sns_sensor *const this);
sns_rc demo_md_deinit(sns_sensor *const this);
sns_rc demo_temp_init(sns_sensor *const this);
sns_rc demo_temp_deinit(sns_sensor *const this);


extern sns_sensor_api demo_acc_sensor_api;
// extern sns_sensor_api demo_acc_sensor_instance_api;
extern sns_sensor_api demo_gyro_sensor_api;
// extern sns_sensor_api demo_gyro_sensor_instance_api;
extern sns_sensor_api demo_md_sensor_api;
// extern sns_sensor_api demo_md_sensor_instance_api;
extern sns_sensor_api demo_temp_sensor_api;
// extern sns_sensor_api demo_temp_sensor_instance_api;
