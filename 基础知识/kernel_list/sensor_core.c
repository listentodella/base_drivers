#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "list.h"
#include "sensor.h"


static LIST_HEAD(sensor_dev_list);


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
