#include "sns_demo_sensor.h"

// please publish enough attr, otherwise framework will not call set_client_req ...
static void demo_md_publish_attributes(sns_sensor *const this)
{
    {// sensor type
        sns_std_attr_value_data val = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg){.buf = "motion_detect", .buf_len = sizeof("motion_detect")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_TYPE, &val, 1, false);
    }

    demo_publish_default_registry_attributes(this);

    {//TODO: active current, uA
        sns_std_attr_value_data values[] = {
            {.has_sint = true, .sint = 3},
            {.has_sint = true, .sint = 300}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_ACTIVE_CURRENT, values, ARR_SIZE(values), false);
    }

    {// low power current
        sns_std_attr_value_data value = {.has_sint = true, .sint = 3};//uA
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_SLEEP_CURRENT, &value, 1, false);
    }

    {// proto
        sns_std_attr_value_data value = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg) {.buf = "sns_motion_detect.proto", .buf_len = sizeof("sns_motion_detect.proto")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_API, &value, 1, false);
    }

    {// physical sensor tests supported
        sns_std_attr_value_data values[] = {
            {.has_sint = true, .sint = SNS_PHYSICAL_SENSOR_TEST_TYPE_COM}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR_TESTS, values, ARR_SIZE(values), true);
    }

    // {// available now?
    //     sns_std_attr_value_data value = {.has_boolen = true, .boolean = false};
    //     sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_AVAILABLE, &value, 1, false);
    // }
}

/* See sns_sensor::init */
sns_rc demo_md_init(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    state->sensor_idx  = MOTION;
    state->sensor_type = DEMO_MOTION;
    state->hw_idx = this->cb->get_registration_index(this);// idx related with registration_cnt

    sns_memscpy(&state->my_suid, sizeof(state->my_suid), &(suids[MOTION][state->hw_idx]), sizeof(sns_sensor_uid));
    DEMO_SENSOR_LOG(HIGH, this, "hw_idx = %d, suid = %"PRIsuid, state->hw_idx, SNS_PRI_SUID(&state->my_suid));

    demo_common_init(this);
    demo_md_publish_attributes(this);

    return SNS_RC_SUCCESS;
}

sns_rc demo_md_deinit(sns_sensor *const this)
{
#if !DEMO_CONFIG_ENABLE_SEE_LITE
    demo_state *state = (demo_state *)this->state->state;
    UNUSED_VAR(state);
#endif
    DEMO_SENSOR_LOG(MED, this, "demo_temp_deinit");

    return SNS_RC_SUCCESS;
}
