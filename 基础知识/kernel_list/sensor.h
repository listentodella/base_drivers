#ifndef __SENSOR_H_
#define __SENSOR_H_
#include "list.h"
#include <stdint.h>

//assume only 3 axis
typedef struct axis_map {
    uint8_t axis[3];//x,y,z

    //each axis represents as an integer in range[-3, 3], excluding 0
    //1 -> x, 2 -> y, 3 -> z
    //-1 -> -x, -2->-y, -3->-z
    //so if want to map the chip's X to device's -Y, set sign[0] = -2
    int8_t  sign[3];
} axis_map_t;


typedef enum sensor_type {
    ACC,
    GYR,
    MAG,
    TEMP
} sensor_type_t;


typedef struct sensor {
    void *priv_data;
    char name[32];
    sensor_type_t type;
    axis_map_t axis_map;
    struct list_head node;
} sensor_t;



void sensor_list();
int sensor_init(sensor_t *sensor);
int sensor_register(sensor_t *sensor);
int sensor_unregister(sensor_t *sensor);

void sensor_axis_map_init(axis_map_t *axis_map, uint8_t *axis_cfg);
void sensor_data_map_axis(axis_map_t *axis_map, int16_t *data);

#endif
