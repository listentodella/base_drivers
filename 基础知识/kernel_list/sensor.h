#ifndef __SENSOR_H_
#define __SENSOR_H_
#include "list.h"


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
    struct list_head node;
} sensor_t;



void sensor_list();
int sensor_init(sensor_t *sensor);
int sensor_register(sensor_t *sensor);
int sensor_unregister(sensor_t *sensor);


#endif
