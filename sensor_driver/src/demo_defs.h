#ifndef __DEMO_DEFS_H__
#define __DEMO_DEFS_H__
#include "stdint.h"

/* default values */
#define DEMO_CHIP_ID_0                           UINT8_C(0x40)
#define DEMO_CHIP_ID_1                           UINT8_C(0x41)

#define DEMO_SW_RESET                            UINT16_C(0xDEAF)


/* register addr */
#define DEMO_CHIP_ID_ADDR                        UINT8_C(0x00)
#define DEMO_ERR_REG_ADDR                        UINT8_C(0x01)
#define DEMO_STATUS_ADDR                         UINT8_C(0x02)
#define DEMO_ACC_X_ADDR                          UINT8_C(0x03)
#define DEMO_ACC_Y_ADDR                          UINT8_C(0x04)
#define DEMO_ACC_Z_ADDR                          UINT8_C(0x05)
#define DEMO_GYR_X_ADDR                          UINT8_C(0x06)
#define DEMO_GYR_Y_ADDR                          UINT8_C(0x07)
#define DEMO_GYR_Z_ADDR                          UINT8_C(0x08)
#define DEMO_TEMP_ADDR                           UINT8_C(0x09)
#define DEMO_TIME0_ADDR                          UINT8_C(0x0A)
#define DEMO_TIME1_ADDR                          UINT8_C(0x0B)


#define DEMO_INT_STATUS_1_ADDR                   UINT8_C(0x0D)//clear-on-read
#define DEMO_INT_STATUS_2_ADDR                   UINT8_C(0x0E)

#define DEMO_FEATURE_IO0_ADDR                    UINT8_C(0x10)
#define DEMO_FEATURE_IO1_ADDR                    UINT8_C(0x11)
#define DEMO_FEATURE_IO2_ADDR                    UINT8_C(0x12)
#define DEMO_FEATURE_IO3_ADDR                    UINT8_C(0x13)
#define DEMO_FEATURE_IO_STATUS_ADDR              UINT8_C(0x14)

#define DEMO_FIFO_LENGTH_ADDR                    UINT8_C(0x15)//fill state in WORDS
#define DEMO_FIFO_DATA_ADDR                      UINT8_C(0x16)

#define DEMO_ACC_CONF_ADDR                       UINT8_C(0x20)
#define DEMO_GYR_CONF_ADDR                       UINT8_C(0x21)

#define DEMO_ALT_ACC_CONF_ADDR                   UINT8_C(0x28)
#define DEMO_ALT_GYR_CONF_ADDR                   UINT8_C(0x29)
#define DEMO_ALT_CONF_ADDR                       UINT8_C(0x2A)
#define DEMO_ALT_STATUS_ADDR                     UINT8_C(0x2B)
#define DEMO_FIFO_WTM_ADDR                       UINT8_C(0x35)
#define DEMO_FIFO_CONF_ADDR                      UINT8_C(0x36)
#define DEMO_FIFO_CTRL_ADDR                      UINT8_C(0x37)
#define DEMO_INT_IO_CTRL_ADDR                    UINT8_C(0x38)//config the electrical behavior of irq pins
#define DEMO_INT_LATCH_ADDR                      UINT8_C(0x39)//irq latch
#define DEMO_INT_MAP1_FEAT_ADDR                  UINT8_C(0x3A)
#define DEMO_INT_MAP2_FEAT_ADDR                  UINT8_C(0x3B)
#define DEMO_FEATURE_CTRL_ADDR                   UINT8_C(0x40)
#define DEMO_FEATURE_TX_ADDR                     UINT8_C(0x41)
#define DEMO_FEATURE_DATA_ADDR                   UINT8_C(0x42)
#define DEMO_FEATURE_DATA_STATUS_ADDR            UINT8_C(0x43)
#define DEMO_FEATURE_ENGINE_STATUS_ADDR          UINT8_C(0x42)

#define DEMO_DMA_ST_RESULT_ADDR                  UINT8_C(0x24)
#define DEMO_DMA_ST_SELECT_ADDR                  UINT8_C(0x25)


#define DEMO_CMD_ADDR                       UINT8_C(0x7E)






#define DEMO_ACC_CONF_ODR_MASK                UINT8_C(0x0F)
#define DEMO_GYR_CONF_ODR_MASK                UINT8_C(0x0F)



#define DEMO_ODR_0_HZ                 (0.0)
#define DEMO_ODR_12P5_HZ              (12.5)
#define DEMO_ODR_25_HZ                (25.0)
#define DEMO_ODR_50_HZ                (50.0)
#define DEMO_ODR_100_HZ               (100.0)
#define DEMO_ODR_200_HZ               (200.0)
#define DEMO_ODR_400_HZ               (400.0)
#define DEMO_ODR_800_HZ               (800.0)
#define DEMO_ODR_1600_HZ              (1600.0)
#define DEMO_ODR_3200_HZ              (3200.0)
#define DEMO_ODR_6400_HZ              (6400.0)

#define ACC_CONVERSION             (G)//m/s^2
#define ACC_RSL_CONVERSION         (G / 1000.0f)//mG = m/s^2
#define GYRO_CONVERSION            (PI / 180.0f)//dps = rps

// acc resolution ==> mg/LSB
#define DEMO_ACC_RSL_2G            (0.061)
#define DEMO_ACC_RSL_4G            (0.122)
#define DEMO_ACC_RSL_8G            (0.244)
#define DEMO_ACC_RSL_16G           (0.488)
// acc range
#define DEMO_ACC_RANGE_2G_MIN      (-2)
#define DEMO_ACC_RANGE_2G_MAX      (2)
#define DEMO_ACC_RANGE_4G_MIN      (-4)
#define DEMO_ACC_RANGE_4G_MAX      (4)
#define DEMO_ACC_RANGE_8G_MIN      (-8)
#define DEMO_ACC_RANGE_8G_MAX      (8)
#define DEMO_ACC_RANGE_16G_MIN     (-16)
#define DEMO_ACC_RANGE_16G_MAX     (16)
// gyro range
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
// gyro resolutions => degree/s LSB
#define DEMO_GYRO_RSL_125DPS       (250 * 1.0 / 65536)
#define DEMO_GYRO_RSL_250DPS       (500 * 1.0 / 65536)
#define DEMO_GYRO_RSL_500DPS       (1000 * 1.0 / 65536)
#define DEMO_GYRO_RSL_1000DPS      (2000 * 1.0 / 65536)
#define DEMO_GYRO_RSL_2000DPS      (4000 * 1.0 / 65536)

#define DEMO_TEMP_RSL               (0.002)
#define DEMO_TEMP_RANGE_MIN         (-40)
#define DEMO_TEMP_RANGE_MAX         (85)





















#endif