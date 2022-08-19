#include "sns_demo_sensor.h"

// please publish enough attr, otherwise framework will not call set_client_req ...
static void demo_temp_publish_attributes(sns_sensor *const this)
{
    {// sensor type
        sns_std_attr_value_data val = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg){.buf = "sensor_temperature", .buf_len = sizeof("sensor_temperature")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_TYPE, &val, 1, false);
    }

    demo_publish_default_registry_attributes(this);

    {//
        sns_std_attr_value_data values[] = {
            {.has_flt = true, .flt = DEMO_TEMP_RSL},
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RESOLUTIONS, values, ARR_SIZE(values), false);
    }

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
            .str.arg = &((pb_buffer_arg) {.buf = "sns_sensor_temperature.proto", .buf_len = sizeof("sns_sensor_temperature.proto")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_API, &value, 1, false);
    }

    {// physical sensor tests supported
        sns_std_attr_value_data values[] = {
            {.has_sint = true, .sint = SNS_PHYSICAL_SENSOR_TEST_TYPE_COM}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR_TESTS, values, ARR_SIZE(values), true);
    }

    {// range supported
        sns_std_attr_value_data values[] = {SNS_ATTR};
        sns_std_attr_value_data ranges[][2] = {
            {{.has_flt = true, .flt = DEMO_TEMP_RANGE_MIN}, {.has_flt = true, .flt = DEMO_TEMP_RANGE_MAX}}
        };

        pb_buffer_arg pb_arg[ARR_SIZE(values)];
        for (uint8_t i = 0; i < ARR_SIZE(values); i++) {
            values[i].has_subtype = true;
            values[i].subtype.values.funcs.encode = sns_pb_encode_attr_cb;
            pb_arg[i].buf = ranges[i];
            pb_arg[i].buf_len = ARR_SIZE(ranges[i]);
            values[i].subtype.values.arg = &pb_arg[i];
            // values[i].subtype.values.arg = &((pb_buffer_arg){.buf = ranges[i], .buf_len = ARR_SIZE(ranges[i])});
        }
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RANGES, values, ARR_SIZE(values), false);
    }

    {// odr supported
        sns_std_attr_value_data values[] = {
            {.has_flt = true, .flt = 1.0},
            {.has_flt = true, .flt = 5.0},
        };

        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RATES, values, ARR_SIZE(values), false);
    }

    // {// available now?
    //     sns_std_attr_value_data value = {.has_boolen = true, .boolean = false};
    //     sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_AVAILABLE, &value, 1, false);
    // }
}

/* See sns_sensor::init */
sns_rc demo_temp_init(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    state->sensor_idx  = TEMP;
    state->sensor_type = DEMO_TEMP;
    state->hw_idx = this->cb->get_registration_index(this);// idx related with registration_cnt

    sns_memscpy(&state->my_suid, sizeof(state->my_suid), &(suids[TEMP][state->hw_idx]), sizeof(sns_sensor_uid));
    DEMO_SENSOR_LOG(HIGH, this, "hw_idx = %d, suid = %"PRIsuid, state->hw_idx, SNS_PRI_SUID(&state->my_suid));

    state->scale_factor = TEMP_FACTOR;//TODO:is same as sns_math_util.h?
    demo_common_init(this);
    demo_temp_publish_attributes(this);

    return SNS_RC_SUCCESS;
}

sns_rc demo_temp_deinit(sns_sensor *const this)
{
#if !DEMO_CONFIG_ENABLE_SEE_LITE
    demo_state *state = (demo_state *)this->state->state;
    UNUSED_VAR(state);
#endif
    DEMO_SENSOR_LOG(MED, this, "demo_temp_deinit");

    return SNS_RC_SUCCESS;
}
