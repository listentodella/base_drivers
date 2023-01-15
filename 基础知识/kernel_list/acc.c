#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "sensor.h"



int main()
{
    sensor_t acc = {
        .type = ACC,
        .name = "bst"
    };
    sensor_register(&acc);

    sensor_t gyr = {
        .type = GYR,
        .name = "qst"
    };
    sensor_register(&gyr);

    //diy:x = -y, y = -x, z = -z
    int8_t cfg[3] = {-2, -1, -3};

    sensor_axis_map_init(&acc.axis_map, cfg);

    sensor_list();

    printf("acc axismap [%d, %d, %d] sign [%d, %d, %d]\n",
            acc.axis_map.axis[0],
            acc.axis_map.axis[1],
            acc.axis_map.axis[2],
            acc.axis_map.sign[0],
            acc.axis_map.sign[1],
            acc.axis_map.sign[2]
          );

    int16_t data[3] = {1, 2, 10};
    printf("get raw data [%d, %d, %d]\n", data[0], data[1], data[2]);

    sensor_data_map_axis(&acc.axis_map, data);
    printf("remap data [%d, %d, %d]\n", data[0], data[1], data[2]);


    //sensor_unregister(&gyr);
    //sensor_list();


    return 0;
}
