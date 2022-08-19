#include "sns_demo_sensor.h"

sns_sensor_uid suids[][2] = {
    {ACCEL_SUID_0, ACCEL_SUID_1},
    {GYRO_SUID_0, GYRO_SUID_1},
    {MD_SUID_0, MD_SUID_1},
    {TEMP_SUID_0, TEMP_SUID_1}
};

// please publish enough attr, otherwise framework will not call set_client_req ...
static void demo_acc_publish_attributes(sns_sensor *const this)
{
    {// sensor type
        sns_std_attr_value_data val = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg){.buf = "accel", .buf_len = sizeof("accel")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_TYPE, &val, 1, false);
    }

    demo_publish_default_registry_attributes(this);

    {// fastest odr
        // Low latency channel use same ODR as traditional channel, no additional
        // rate to publish, if any additional rate to be supported by low latency
        // channel, please publish here.

        // Following is for example only and exercising low latency path via QC example is TBD
        sns_std_attr_value_data values[] = {{.has_flt = true, .flt = DEMO_ODR_400_HZ}};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_ADDITIONAL_LOW_LATENCY_RATES, values, ARR_SIZE(values), false);
    }

    {// range/resolution supported, mg/LSB
        sns_std_attr_value_data values[] = {
            {.has_flt = true, .flt = DEMO_ACC_RSL_2G},
            {.has_flt = true, .flt = DEMO_ACC_RSL_4G},
            {.has_flt = true, .flt = DEMO_ACC_RSL_8G},
            {.has_flt = true, .flt = DEMO_ACC_RSL_16G}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RESOLUTIONS, values, ARR_SIZE(values), false);
    }

    {//TODO: active current, uA
        sns_std_attr_value_data values[] = {
            {.has_sint = true, .sint = 3},
            {.has_sint = true, .sint = 180}
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
            .str.arg = &((pb_buffer_arg) {.buf = "sns_acc.proto", .buf_len = sizeof("sns_acc.proto")})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_API, &value, 1, false);
    }

    {// physical sensor tests supported
        sns_std_attr_value_data values[] = {
            {.has_sint = true, .sint = SNS_PHYSICAL_SENSOR_TEST_TYPE_COM},
            {.has_sint = true, .sint = SNS_PHYSICAL_SENSOR_TEST_TYPE_FACTORY},
            {.has_sint = true, .sint = SNS_PHYSICAL_SENSOR_TEST_TYPE_HW}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR_TESTS, values, ARR_SIZE(values), true);
    }

    {// range supported
        sns_std_attr_value_data values[] = {SNS_ATTR, SNS_ATTR, SNS_ATTR, SNS_ATTR};
        sns_std_attr_value_data ranges[][2] = {
            {{.has_flt = true, .flt = DEMO_ACC_RANGE_2G_MIN * ACC_CONVERSION},
             {.has_flt = true, .flt = DEMO_ACC_RANGE_2G_MAX * ACC_CONVERSION}},
            {{.has_flt = true, .flt = DEMO_ACC_RANGE_4G_MIN * ACC_CONVERSION},
             {.has_flt = true, .flt = DEMO_ACC_RANGE_4G_MAX * ACC_CONVERSION}},
            {{.has_flt = true, .flt = DEMO_ACC_RANGE_8G_MIN * ACC_CONVERSION},
             {.has_flt = true, .flt = DEMO_ACC_RANGE_8G_MAX * ACC_CONVERSION}},
            {{.has_flt = true, .flt = DEMO_ACC_RANGE_16G_MIN * ACC_CONVERSION},
             {.has_flt = true, .flt = DEMO_ACC_RANGE_16G_MAX * ACC_CONVERSION}}
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
            {.has_flt = true, .flt = DEMO_ODR_12P5_HZ},
            {.has_flt = true, .flt = DEMO_ODR_25_HZ},
            {.has_flt = true, .flt = DEMO_ODR_50_HZ},
            {.has_flt = true, .flt = DEMO_ODR_100_HZ},
            {.has_flt = true, .flt = DEMO_ODR_200_HZ},
            {.has_flt = true, .flt = DEMO_ODR_400_HZ},
            {.has_flt = true, .flt = DEMO_ODR_800_HZ},
            {.has_flt = true, .flt = DEMO_ODR_1600_HZ},
            {.has_flt = true, .flt = DEMO_ODR_3200_HZ},
            {.has_flt = true, .flt = DEMO_ODR_6400_HZ}
        };

        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RATES, values, ARR_SIZE(values), false);
    }

    // {// available now?
    //     sns_std_attr_value_data value = {.has_boolen = true, .boolean = false};
    //     sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_AVAILABLE, &value, 1, false);
    // }
}

/* See sns_sensor::init */
sns_rc demo_acc_init(sns_sensor *const this)
{
    demo_state *state = (demo_state *) this->state->state;
    state->sensor_idx  = ACC;
    state->sensor_type = DEMO_ACCEL;
    state->hw_idx = this->cb->get_registration_index(this);// idx related with registration_cnt

    sns_memscpy(&state->my_suid, sizeof(state->my_suid), &(suids[ACC][state->hw_idx]), sizeof(sns_sensor_uid));
    DEMO_SENSOR_LOG(HIGH, this, "hw_idx = %d, suid = %"PRIsuid, state->hw_idx, SNS_PRI_SUID(&state->my_suid));

    float resolutions[] = {DEMO_ACC_RSL_2G, DEMO_ACC_RSL_4G, DEMO_ACC_RSL_8G, DEMO_ACC_RSL_16G};//mg/LSB
    sns_memscpy(&state->resolutions, sizeof(state->resolutions), resolutions, sizeof(state->resolutions));
    //why need this factor since later offset it
    //state->scale_factor = DEMO_SCALE_FACTOR_DATA_ACCEL * 1e-6 * G;
    state->scale_factor = G * 1e-3;

    demo_common_init(this);
    demo_acc_publish_attributes(this);

    return SNS_RC_SUCCESS;
}

sns_rc demo_acc_deinit(sns_sensor *const this)
{
#if !DEMO_CONFIG_ENABLE_SEE_LITE
    demo_state *state = (demo_state *)this->state->state;
    UNUSED_VAR(state);
#endif
    DEMO_SENSOR_LOG(MED, this, "demo_acc_deinit");

    return SNS_RC_SUCCESS;
}
