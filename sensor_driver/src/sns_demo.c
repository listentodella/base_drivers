#include "sns_rc.h"
#include "sns_types.h"
#include "sns_register.h"
#include "sns_demo_sensor.h"
#include "sns_demo_sensor_instance.h"

sns_rc sns_register_demo(sns_register_cb const *register_api)
{
    //why use sizeof(demo_state) ?
    // maybe it's related with zero-length array --> sns_sensor/intance_state
    // NOTE! each sensor owns sns_sensor data struct, same name but different var
    // if you change the seq of init_sensor, don't forget the first one should send_registry req
    // for pf,etc.
    register_api->init_sensor(sizeof(demo_state), &demo_acc_sensor_api, &demo_sensor_instance_api);
    register_api->init_sensor(sizeof(demo_state), &demo_gyro_sensor_api, &demo_sensor_instance_api);
    register_api->init_sensor(sizeof(demo_state), &demo_md_sensor_api, &demo_sensor_instance_api);
    register_api->init_sensor(sizeof(demo_state), &demo_temp_sensor_api, &demo_sensor_instance_api);

    return SNS_RC_SUCCESS;
}