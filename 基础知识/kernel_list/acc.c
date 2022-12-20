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

    sensor_list();

    //sensor_unregister(&gyr);
    //sensor_list();


    return 0;
}
