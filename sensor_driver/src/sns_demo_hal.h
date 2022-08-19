#pragma once
#include "demo_defs.h"
#include "sns_demo_common.h"
// #include "sns_demo_sensor.h"
// #include "sns_demo_sensor_instance.h"

#if 0
/**
 * DEMO ODR (Hz) definitions
 */
#define DEMO_ODR_0                 0.0
#define DEMO_ODR_12P5              12.5
#define DEMO_ODR_25                25.0
#define DEMO_ODR_50                50
#define DEMO_ODR_100               100
#define DEMO_ODR_200               200
#define DEMO_ODR_400               400
#define DEMO_ODR_800               800
#define DEMO_ODR_1600              1600
#define DEMO_ODR_3200              3200.0
#define DEMO_ODR_6400              6400.0

/**
 * Accelerometer ranges
 */
#define DEMO_ACCEL_RANGE_2G_MIN    (-2)
#define DEMO_ACCEL_RANGE_2G_MAX    (2)
#define DEMO_ACCEL_RANGE_4G_MIN    (-4)
#define DEMO_ACCEL_RANGE_4G_MAX    (4)
#define DEMO_ACCEL_RANGE_8G_MIN    (-8)
#define DEMO_ACCEL_RANGE_8G_MAX    (8)
#define DEMO_ACCEL_RANGE_16G_MIN   (-16)
#define DEMO_ACCEL_RANGE_16G_MAX   (16)

/**
 * Accelerometer resolutions
 */
#define DEMO_ACCEL_RES_2G    (0.061)
#define DEMO_ACCEL_RES_4G    (0.122)
#define DEMO_ACCEL_RES_8G    (0.244)
#define DEMO_ACCEL_RES_16G   (0.488)

/**
 * Gyroscope ranges
 */
#define DEMO_GYRO_RANGE_125_MIN    (-125)
#define DEMO_GYRO_RANGE_125_MAX    (125)
#define DEMO_GYRO_RANGE_250_MIN    (-250)
#define DEMO_GYRO_RANGE_250_MAX    (250)
#define DEMO_GYRO_RANGE_500_MIN    (-500)
#define DEMO_GYRO_RANGE_500_MAX    (500)
#define DEMO_GYRO_RANGE_1000_MIN   (-1000)
#define DEMO_GYRO_RANGE_1000_MAX   (1000)
#define DEMO_GYRO_RANGE_2000_MIN   (-2000)
#define DEMO_GYRO_RANGE_2000_MAX   (2000)



#define DEMO_GYRO_RES_125DPS    (250 * 1.0 / 65536)
#define DEMO_GYRO_RES_250DPS    (500 * 1.0 / 65536)
#define DEMO_GYRO_RES_500DPS    (1000 * 1.0 / 65536)
#define DEMO_GYRO_RES_1000DPS    (2000 * 1.0 / 65536)
#define DEMO_GYRO_RES_2000DPS    (4000 * 1.0 / 65536)
#endif



typedef struct range_attr {
    float min;
    float max;
} range_attr;

extern const range_attr demo_accel_ranges[];
extern const float demo_accel_res[];
extern const range_attr demo_gyro_ranges[];
extern const float demo_gyro_res[];
extern const range_attr demo_temp_ranges[];
extern const float demo_temp_res[];

extern const range_attr demo_ss_ranges[][5];
extern const float demo_ss_rsl[][5];

sns_rc demo_com_read_wrapper(sns_sync_com_port_service *scp_service,
                             sns_sync_com_port_handle **port_handle,
                            //  sns_sync_com_port_handle *port_handle,
                             uint32_t addr, uint8_t *buffer,
                             uint32_t size, uint32_t *xfer_size);

sns_rc demo_com_write_wrapper(sns_sync_com_port_service *scp_service,
                             sns_sync_com_port_handle *port_handle,
                             uint32_t addr, uint8_t *buffer,
                             uint32_t size, uint32_t *xfer_size,
                             bool save_write_time);

sns_rc demo_switch_bus(sns_bus_type bus_type, sns_sync_com_port_service *scp_service,
                       sns_sync_com_port_handle **port_handle);

void demo_dump_status(sns_sensor_instance *const this);
// void demo_hal_register_interrupt(sns_sensor_instance *this, demo_sensor_type sensor_type);
void demo_hal_register_interrupt(sns_sensor_instance *this, demo_sensor_idx sensor_idx);
sns_rc demo_hal_reset_device(sns_sensor_instance *const instance);
sns_rc demo_read_chip_id(sns_sync_com_port_service *scp_service,
                         sns_sync_com_port_handle **sns_sync_com_port_handle, uint8_t *buffer);
void demo_handle_acc_drdy_irq_event(sns_sensor_instance *const this,
                                 sns_interrupt_event irq_event);
void demo_handle_gyr_drdy_irq_event(sns_sensor_instance *const this,
                                 sns_interrupt_event irq_event);
void demo_handle_wtm_irq_event(sns_sensor_instance *const this,
                                 sns_interrupt_event irq_event);
void demo_handle_polling_timer_event(sns_sensor_instance *const this, demo_sensor_idx sensor_idx,
                                 sns_timer_sensor_event *const timer_event);
void demo_handle_gyr_timer_event(sns_sensor_instance *const this,
                                 sns_timer_sensor_event *const timer_event);
void demo_handle_temp_timer_event(sns_sensor_instance *const this,
                                 sns_timer_sensor_event *const timer_event);

void demo_send_flush_done(sns_sensor_instance *instance, demo_sensor_type flush_sensors,
                          flush_done_reason reason);

static inline void udelay(uint32_t us)
{
    sns_time ticks;
    ticks = (sns_time)sns_convert_ns_to_ticks((unsigned)(us * 1000));
    sns_busy_wait(ticks);
}
