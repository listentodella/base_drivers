#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "list.h"
#include "sensor.h"


static LIST_HEAD(sensor_dev_list);

#define SIGN_OF_INT8(x) (1 | ((x) >> 7))
void sensor_axis_map_init(axis_map_t *axis_map, uint8_t *axis_cfg_data)
{
    const int8_t *axis_cfg = (int8_t *)axis_cfg_data;
    //default cfg
    if (!axis_cfg) {
        axis_map->axis[0] = 0;
        axis_map->axis[1] = 1;
        axis_map->axis[2] = 2;
        axis_map->sign[0] = 1;
        axis_map->sign[1] = 1;
        axis_map->sign[2] = 1;
        printf("use default cfg\n");
    } else {
        printf("use diy cfg\n");
        //convert cfg to int first, then select its sign bit
        //so value is +1 or -1
        axis_map->sign[0] = SIGN_OF_INT8((axis_cfg[0]));
        axis_map->axis[0] = axis_cfg[0] * axis_map->sign[0] - 1;

        axis_map->sign[1] = SIGN_OF_INT8((axis_cfg[1]));
        axis_map->axis[1] = axis_cfg[1] * axis_map->sign[1] - 1;

        axis_map->sign[2] = SIGN_OF_INT8((axis_cfg[2]));
        axis_map->axis[2] = axis_cfg[2] * axis_map->sign[2] - 1;
    }

    return;
}


void sensor_data_map_axis(axis_map_t *axis_map, int16_t *data)
{
    int16_t tmp[3] = {0};
    tmp[0] = data[axis_map->axis[0]] * axis_map->sign[0];
    tmp[1] = data[axis_map->axis[1]] * axis_map->sign[1];
    tmp[2] = data[axis_map->axis[2]] * axis_map->sign[2];
    data[0] = tmp[0];
    data[1] = tmp[1];
    data[2] = tmp[2];
}


int sensor_init(sensor_t *sensor)
{
    int ret = 0;


    return ret;
}

int sensor_register(sensor_t *sensor)
{
    int ret = 0;

    list_add_tail(&sensor->node, &sensor_dev_list);

    return ret;
}


int sensor_unregister(sensor_t *sensor)
{
    int ret = 0;
    //before we remove this node, please check whether need disconnect something else

    //we just need remove this node
    list_del_init(&sensor->node);

    return ret;
}

void sensor_list()
{
    sensor_t *sensor, *tmp;
    list_for_each_entry_safe(sensor, tmp, &sensor_dev_list, node) {
        printf("get sensor %d, name %s\n", sensor->type, sensor->name);
    }
}
