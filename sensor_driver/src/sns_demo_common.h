#pragma once
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "sns_rc.h"//return value
#include "sns_types.h"// ARRAY_SIZE / UNUSED_VAR
// #include "sns_printf_int.h"
#include "sns_printf.h"// print log
#include "sns_register.h" // register driver into sensor library entry
#include "sns_service.h"// service type
#include "sns_service_manager.h"//get service manager
#include "sns_diag_service.h"
#include "sns_gpio_service.h"// read/write gpio
#include "sns_event_service.h"
#include "sns_stream_service.h"
#include "sns_island_service.h"
#include "sns_com_port_types.h"// bus type, common info
#include "sns_sync_com_port_service.h" // bus read / write
#include "sns_pwr_rail_service.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "sns_time.h"
#include "sns_sensor.h"
#include "sns_sensor_instance.h"
#include "sns_request.h"
#include "sns_sensor_event.h"

/* util common */
#include "sns_pb_util.h"
#include "sns_mem_util.h"// sns_memxxx
#include "sns_sensor_util.h"
#include "sns_cal_util.h"
#include "sns_suid_util.h"
#include "sns_math_util.h"
#include "sns_registry_util.h"
#include "sns_attribute_util.h"
#include "sns_async_com_port_pb_utils.h"

/* .pb.h */
#include "sns_std.pb.h"
#include "sns_suid.pb.h"
#include "sns_timer.pb.h"
#include "sns_registry.pb.h"
#include "sns_interrupt.pb.h"
#include "sns_std_sensor.pb.h"
#include "sns_async_com_port.pb.h"

#include "sns_motion_detect.pb.h"
#include "sns_physical_sensor_test.pb.h"
#include "sns_std_event_gated_sensor.pb.h"

#include "sns_cal.pb.h"
//#if DEMO_CONFIG_ENABLE_DIAG_LOG
#include "sns_diag.pb.h"
//#endif
#define DEMO_DEBUG_PRINT  (1)
#if DEMO_DEBUG_PRINT
extern void sns_diag_sensor_sprintf(uint16_t ssid, /*!< DIAG Subsystem ID */
                                    const sns_sensor *sensor, sns_message_priority prio,
                                    const char *file, uint32_t line, const char *format, ...);
extern sns_sensor *sns_fw_printf;

#define SNS_SPRINTF(prio, sensor, fmt, ...)                                                     \
    do {                                                                                        \
        sns_diag_sensor_sprintf(SNS_DIAG_SSID, sensor, SNS_##prio, __FILENAME__, __LINE__, "%s:"fmt, \
                                __func__, ##__VA_ARGS__);                                       \
    } while (0)
#endif

#define DEMO_CONFIG_DFT_LOG_LEVEL   SNS_LOW

#if !DEMO_DEBUG_PRINT
#define DEMO_SENSOR_LOG(LOG_LEVEL, this, arg...) { \
    if (NULL != this) { \
        if (SNS_##LOG_LEVEL >= DEMO_CONFIG_DFT_LOG_LEVEL) { \
            SNS_PRINTF(LOG_LEVEL, this, ##arg); \
        } \
    } \
}

#define DEMO_INSTANCE_LOG(LOG_LEVEL, this, arg...) { \
    if (NULL != this) { \
        if (SNS_##LOG_LEVEL >= DEMO_CONFIG_DFT_LOG_LEVEL) { \
            SNS_INST_PRINTF(LOG_LEVEL, this, ##arg); \
        } \
    } \
}
#else
#define DEMO_SENSOR_LOG(LOG_LEVEL, this, arg...) { \
    if (NULL != this) { \
        if (SNS_##LOG_LEVEL >= DEMO_CONFIG_DFT_LOG_LEVEL) { \
            SNS_SPRINTF(LOG_LEVEL, sns_fw_printf, ##arg); \
        } \
    } \
}

#define DEMO_INSTANCE_LOG(LOG_LEVEL, this, arg...) { \
    if (NULL != this) { \
        if (SNS_##LOG_LEVEL >= DEMO_CONFIG_DFT_LOG_LEVEL) { \
            SNS_SPRINTF(LOG_LEVEL, sns_fw_printf, ##arg); \
        } \
    } \
}
#endif


// #define my_offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)
// #define container_of(ptr, type, member) ({                      \
	// const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	// (type *)( (char *)__mptr - offsetof(type,member) );})
#define container_of(member_type, member_ptr, type, member) \
    ({                                                      \
        member_type *__mptr = (member_ptr);                 \
        (type *)((char *)__mptr - offsetof(type, member));  \
    })
#if 0
//error: arithmetic on a pointer to void is a GNU extension
#define container_of_v2(ptr, type, member)           \
    ({                                               \
        void *__mptr = (void *)(ptr);                \
        ((type *)(__mptr - offsetof(type, member))); \
    })
#endif
// #define DALSYSCMN_CONTAINEROF(ptr, type, member)  \
//    ({const typeof( ((type *)0)->member ) *__mptr = (ptr);  (type *)( (char *)__mptr - offsetof(type,member) );})

// #define SENSOR_HZ(_hz)          ((uint32_t)((_hz) * 1024.0f))
#define SENSOR_HZ(_hz)          ((uint32_t)((_hz)))
#define GET_LSB(var)            (uint8_t)(var & 0x00FF)
#define GET_MSB(var)            (uint8_t)(var & 0xFF00)
#define IS_ZERO(x)              (((x) >= -FLT_MIN) && ((x) <= FLT_MIN))

typedef enum {
    DEMO_CMD_SOFT_RESET = 0,
    DEMO_CMD_SELF_TEST = 1,
    DEMO_CMD_UNKNOWN = 2
} demo_cmd_t;

typedef struct com_port_info {
    uint8_t i2c_addr;
    uint8_t i3c_addr;
    uint8_t dummy_byte;
    sns_com_port_config com_config;
    sns_sync_com_port_handle *port_handle;
} com_port_info;

typedef struct irq_info {
    union {
        sns_ibi_req ibi_config;
        sns_interrupt_req irq_config;
    };
    bool irq_registered : 1;//should defined here?
    bool irq_ready : 1;
    bool is_ibi : 1;
} irq_info_t;


typedef enum {
    FLUSH_TO_BE_DONE,           // 0
    FLUSH_DONE_CONFIGURING,     // 1
    FLUSH_DONE_NOT_ACCEL_GYRO,  // 2
    FLUSH_DONE_NOT_FIFO,        // 3
    FLUSH_DONE_FIFO_EMPTY,      // 4
    FLUSH_DONE_AFTER_DATA,      // 5
} flush_done_reason;

typedef enum {
    ACC    = 0,
    GYR    = 1,
    MOTION = 2,
    TEMP   = 3,
    NUMS
} demo_sensor_idx;

typedef enum {
    DEMO_ACCEL   = (1 << ACC),
    DEMO_GYRO    = (1 << GYR),
    DEMO_MOTION  = (1 << MOTION),
    DEMO_TEMP    = (1 << TEMP),
    DEMO_UNKNOWN = (1 << NUMS),
} demo_sensor_type;

typedef struct demo_state demo_state;
typedef struct demo_common_state demo_common_state;

typedef sns_registry_phy_sensor_cfg ss_cfg_t;
typedef sns_registry_phy_sensor_pf_cfg pf_cfg_t;
typedef sns_registry_md_cfg md_cfg_t;


extern sns_sensor_uid acc_suids[];
extern sns_sensor_uid gyr_suids[];
extern sns_sensor_uid temp_suids[];
extern sns_sensor_uid md_suids[];
extern sns_sensor_uid suids[][2];


