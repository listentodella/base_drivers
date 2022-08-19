#pragma once
// #include "sns_demo_sensor.h"
#include "sns_demo_common.h"




typedef struct _demo_self_test_info {
    uint32_t test_type : 4;  // of type sns_physical_sensor_test_type ;

    uint32_t sensor : 8;               // of type demo_sensor_type
    uint32_t test_client_present : 1;  // of type bool
    // uint32_t                    hw_detect_success  :1;//of type bool
} demo_self_test_info;

typedef struct demo_fifo_info {
    uint16_t wtm;
    /* Determines which Sensor data to publish. Use demo_sensor_type as bit mask. */
    // uint32_t publish_sensors;
    uint32_t fifo_enabled;
} demo_fifo_info_t;

typedef struct demo_ss_cfg {
    sns_sensor_uid suid;
    ss_cfg_t ss_cfg;
    size_t encoded_event_len;
    uint16_t wtm;
    float sample_rate;
    float report_rate;

    bool enabled;
    bool timer_is_active;
    float requested_odr;
    float current_odr;
    uint32_t report_timer_hz;
    sns_time sample_intvl;
    sns_time sample_intvl_req;
    sns_data_stream *timer_stream;
} demo_ss_cfg_t;

typedef struct demo_req_payload {
    demo_sensor_idx  sensor_idx;
    demo_sensor_type sensor_type;
    //tdb
    uint32_t sample_rate;
    uint32_t report_rate;
} demo_req_payload;

typedef struct demo_instance_state {
    demo_state *state;
    bool reset_done;
    bool force_reset;
    // bool irq_registered;
    demo_ss_cfg_t ss_cfgs[NUMS];
    demo_fifo_info_t fifo_cfg;
#if 0
    /* data */
    demo_common_state *common;
    sns_sensor_uid registry_suid;

    /* data stream */
    sns_data_stream *suid_stream;
    sns_data_stream *registry_stream;
#endif
    sns_sensor_uid irq_suid;
    sns_sensor_uid ascp_suid;
    sns_sensor_uid timer_suid;

    sns_data_stream *irq_stream;
    sns_data_stream *ascp_stream;
    sns_data_stream *timer_stream;

    /* service */
    //sns_diag_service *diag_service;
    sns_sync_com_port_service *scp_service;
    //sns_pwr_rail_service *pwr_rail_service;
    irq_info_t irq_info;
    com_port_info com_port;
    sns_async_com_port_config ascp_config;

    demo_self_test_info test_info;

    /* sensors being flushed */
    demo_sensor_type flushing_sensors;
    demo_sensor_type sensor_enabled;

} demo_instance_state;







extern sns_sensor_instance_api demo_sensor_instance_api;
sns_rc demo_instance_init(sns_sensor_instance *const this, sns_sensor_state const *state);
sns_rc demo_instance_deinit(sns_sensor_instance *const this);
sns_rc demo_instance_set_client_config(sns_sensor_instance *const this, sns_request const *client_request);



void demo_config_hw(sns_sensor_instance *const this, sns_request const *client_request);
void demo_send_config_event(sns_sensor_instance *const this);
