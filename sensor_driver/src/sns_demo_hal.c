#include "sns_demo_hal.h"



// const range_attr demo_accel_ranges[] = {{DEMO_ACC_RANGE_2G_MIN, DEMO_ACC_RANGE_2G_MAX},
//                                         {DEMO_ACC_RANGE_4G_MIN, DEMO_ACC_RANGE_4G_MAX},
//                                         {DEMO_ACC_RANGE_8G_MIN, DEMO_ACC_RANGE_8G_MAX},
//                                         {DEMO_ACC_RANGE_16G_MIN, DEMO_ACC_RANGE_16G_MAX}};

// const float demo_accel_res[] = {DEMO_ACC_RSL_2G, DEMO_ACC_RSL_4G, DEMO_ACC_RSL_8G,
//                                 DEMO_ACC_RSL_16G};

// const range_attr demo_gyro_ranges[] = {{DEMO_GYRO_RANGE_125_MIN, DEMO_GYRO_RANGE_125_MAX},
//                                        {DEMO_GYRO_RANGE_250_MIN, DEMO_GYRO_RANGE_250_MAX},
//                                        {DEMO_GYRO_RANGE_500_MIN, DEMO_GYRO_RANGE_500_MAX},
//                                        {DEMO_GYRO_RANGE_1000_MIN, DEMO_GYRO_RANGE_1000_MAX},
//                                        {DEMO_GYRO_RANGE_2000_MIN, DEMO_GYRO_RANGE_2000_MAX}};

// const float demo_gyro_res[] = {DEMO_GYRO_RSL_125DPS, DEMO_GYRO_RSL_250DPS, DEMO_GYRO_RSL_500DPS,
//                                DEMO_GYRO_RSL_1000DPS, DEMO_GYRO_RSL_2000DPS};


// const range_attr demo_temp_ranges[] = {{DEMO_TEMP_RANGE_MIN, DEMO_TEMP_RANGE_MAX}};
// const float demo_temp_res[] = {DEMO_ACC_RSL_2G};

const range_attr demo_ss_ranges[NUMS][5] = {
    [ACC] = {{DEMO_ACC_RANGE_2G_MIN, DEMO_ACC_RANGE_2G_MAX},
             {DEMO_ACC_RANGE_4G_MIN, DEMO_ACC_RANGE_4G_MAX},
             {DEMO_ACC_RANGE_8G_MIN, DEMO_ACC_RANGE_8G_MAX},
             {DEMO_ACC_RANGE_16G_MIN, DEMO_ACC_RANGE_16G_MAX}},
    [GYR] = {{DEMO_GYRO_RANGE_125_MIN, DEMO_GYRO_RANGE_125_MAX},
             {DEMO_GYRO_RANGE_250_MIN, DEMO_GYRO_RANGE_250_MAX},
             {DEMO_GYRO_RANGE_500_MIN, DEMO_GYRO_RANGE_500_MAX},
             {DEMO_GYRO_RANGE_1000_MIN, DEMO_GYRO_RANGE_1000_MAX},
             {DEMO_GYRO_RANGE_2000_MIN, DEMO_GYRO_RANGE_2000_MAX}},
    [TEMP] = {{DEMO_TEMP_RANGE_MIN, DEMO_TEMP_RANGE_MAX}},
};

const float demo_ss_rsl[][5] = {
    [ACC] = {DEMO_ACC_RSL_2G, DEMO_ACC_RSL_4G, DEMO_ACC_RSL_8G, DEMO_ACC_RSL_16G},
    [GYR] = {DEMO_GYRO_RSL_125DPS, DEMO_GYRO_RSL_250DPS, DEMO_GYRO_RSL_500DPS,
             DEMO_GYRO_RSL_1000DPS, DEMO_GYRO_RSL_2000DPS},
    [TEMP] = {DEMO_ACC_RSL_2G},
};
