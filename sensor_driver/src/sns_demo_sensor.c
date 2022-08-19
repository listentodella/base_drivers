#include "sns_demo_sensor.h"
// #include "sns_demo_common.h"

#define CONFIG_SPEC             ".demo_spec.dummy_byte"
#define CONFIG_ACCEL            ".accel.config"
#define CONFIG_GYRO             ".gyro.config"
#define CONFIG_TEMP             ".temp.config"
#define CONFIG_MD               ".md.config"
#define PLATFORM_CONFIG         "_platform.config"
#define PLATFORM_PLACEMENT      "_platform.placement"
#define PLATFORM_ORIENT         "_platform.orient"
#define PLATFORM_FAC_CAL_ACCEL  "_platform.accel.fac_cal"
#define PLATFORM_FAC_CAL_GYRO  "_platform.gyro.fac_cal"
#define PLATFORM_FAC_CAL_TEMP   "_platform.temp.fac_cal"
#define PLATFORM_CONFIG_MD      "_platform.md.config"

#define DEMO_GEN_GROUP(x, group) SENSOR_NAME "_" #x group

enum {
    REG_CONFIG_SPEC,
    REG_CONFIG_ACCEL,
    REG_CONFIG_GYRO,
    REG_CONFIG_TEMP,
    REG_CONFIG_MD,
    REG_PLATFORM_CONFIG,
    REG_PLATFORM_PLACEMENT,
    REG_PLATFORM_ORIENT,
    REG_PLATFORM_FAC_CAL_ACCEL,
    REG_PLATFORM_FAC_CAL_GYRO,
    REG_PLATFORM_FAC_CAL_TEMP,
    REG_PLATFORM_CONFIG_MD,
    REG_MAX_CONFIGS,
};

static char *demo_registry_cfg[][REG_MAX_CONFIGS] = {
    {
        DEMO_GEN_GROUP(0, CONFIG_SPEC),
        DEMO_GEN_GROUP(0, CONFIG_ACCEL),
        DEMO_GEN_GROUP(0, CONFIG_GYRO),
        DEMO_GEN_GROUP(0, CONFIG_TEMP),
        DEMO_GEN_GROUP(0, CONFIG_MD),
        DEMO_GEN_GROUP(0, PLATFORM_CONFIG),
        DEMO_GEN_GROUP(0, PLATFORM_PLACEMENT),
        DEMO_GEN_GROUP(0, PLATFORM_ORIENT),
        DEMO_GEN_GROUP(0, PLATFORM_FAC_CAL_ACCEL),
        DEMO_GEN_GROUP(0, PLATFORM_FAC_CAL_GYRO),
        DEMO_GEN_GROUP(0, PLATFORM_FAC_CAL_TEMP),
        DEMO_GEN_GROUP(0, PLATFORM_CONFIG_MD),
    },
    {
        DEMO_GEN_GROUP(1, CONFIG_SPEC),
        DEMO_GEN_GROUP(1, CONFIG_ACCEL),
        DEMO_GEN_GROUP(1, CONFIG_GYRO),
        DEMO_GEN_GROUP(1, CONFIG_TEMP),
        DEMO_GEN_GROUP(1, CONFIG_MD),
        DEMO_GEN_GROUP(1, PLATFORM_CONFIG),
        DEMO_GEN_GROUP(1, PLATFORM_PLACEMENT),
        DEMO_GEN_GROUP(1, PLATFORM_ORIENT),
        DEMO_GEN_GROUP(1, PLATFORM_FAC_CAL_ACCEL),
        DEMO_GEN_GROUP(1, PLATFORM_FAC_CAL_GYRO),
        DEMO_GEN_GROUP(1, PLATFORM_FAC_CAL_TEMP),
        DEMO_GEN_GROUP(1, PLATFORM_CONFIG_MD),
    }
};

static const char ss_name[] = SENSOR_NAME;
static const char ss_vendor[] = VENDOR_NAME;

demo_common_state g_common[2];

static bool demo_decode_std_sensor_config_request(sns_sensor const *this,
                                                  sns_request const *in_request,
                                                  sns_std_request *decoded_request,
                                                  sns_std_sensor_config *decoded_payload)
{
    pb_istream_t stream;
    /* decode argument */
    pb_simple_cb_arg arg = {
        .decoded_struct = decoded_payload,
        .fields = sns_std_sensor_config_event_fields
    };
    /* decode functions.decode */
    decoded_request->payload = (struct pb_callback_s) {
        .funcs.decode = pb_decode_simple_cb,
        .arg = &arg
    };

    stream = pb_istream_from_buffer(in_request->request, in_request->request_len);
    if (!pb_decode(&stream, sns_std_request_fields, decoded_request)) {
        DEMO_SENSOR_LOG(HIGH, this, "decode error for sensor config!");
        return false;
    }
    return true;
}

bool demo_decode_sensor_test_config_request(sns_sensor const *this, sns_request const *in_request,
                                            sns_std_request *decoded_request,
                                            sns_physical_sensor_test_config *decoded_payload)
{
    pb_istream_t stream;
    /* decode argument */
    pb_simple_cb_arg arg = {
        .decoded_struct = decoded_payload,
        .fields = sns_physical_sensor_test_config_fields
    };
    /* decode functions.decode */
    decoded_request->payload = (struct pb_callback_s) {
        .funcs.decode = pb_decode_simple_cb,
        .arg = &arg
    };

    stream = pb_istream_from_buffer(in_request->request, in_request->request_len);
    if (!pb_decode(&stream, sns_std_request_fields, decoded_request)) {
        DEMO_SENSOR_LOG(HIGH, this, "decode error for self test!");
        return false;
    }
    return true;
}

static void demo_get_sensor_config(sns_sensor *const this, sns_sensor_instance *instance,
                                   float *chosen_sample_rate, float *chosen_report_rate,
                                   bool *sensor_client_present)
{
    sns_request const *request;
    sns_std_request decoded_request;
    sns_std_sensor_config decoded_payload = {0};
    demo_state *state           = (demo_state *)this->state->state;
    demo_instance_state *istate = (demo_instance_state *)instance->state->state;

    DEMO_SENSOR_LOG(HIGH, this, "istate = 0x%p, sensor %d, suid = %"PRIsuid, istate,
                    state->sensor_type, SNS_PRI_SUID(&state->my_suid));
    for (request = instance->cb->get_client_request(instance, &state->my_suid, true); request;
         request = instance->cb->get_client_request(instance, &state->my_suid, false)) {
        // sns_std_request decoded_request;
        // sns_std_sensor_config decoded_payload = {0};
        if (request->message_id == SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_CONFIG) {
            if (demo_decode_std_sensor_config_request(this, request, &decoded_request, &decoded_payload)) {
                DEMO_SENSOR_LOG(HIGH, this, "decoded_payload.sample_rate*100 = %d", (uint32_t)decoded_payload.sample_rate*100);
                float report_rate = 0.0f;
                *chosen_sample_rate = SNS_MAX(*chosen_sample_rate, decoded_payload.sample_rate);
                if (decoded_request.has_batching && decoded_request.batching.batch_period > 0) {
                    report_rate = (1e6 / (float)decoded_request.batching.batch_period);
                } else {
                    //report_rate = *chosen_sample_rate;
                    report_rate = decoded_payload.sample_rate;
                }
                *chosen_report_rate = SNS_MAX(*chosen_report_rate, report_rate);
                *sensor_client_present = true;
            }
        } else {
            DEMO_SENSOR_LOG(HIGH, this, "unknown req msg id = %d", request->message_id);
        }
    }
    DEMO_SENSOR_LOG(HIGH, this, "istate = 0x%p, sensor_type = %d, sample_rate = %d",
                    istate, state->sensor_type, (uint32_t)(*chosen_sample_rate));
}

/* get instance config and set_client_config */
void demo_reval_instance_config(sns_sensor *const this, sns_sensor_instance *instance)
{
    demo_state *state           = (demo_state *)this->state->state;
    demo_instance_state *istate = (demo_instance_state *)instance->state->state;

    float chosen_sample_rate = 0.0f;
    float chosen_report_rate = 0.0f;
    bool  sample_rate_need_change = false;
    bool  sensor_client_present = false;

    demo_req_payload req_payload = {
        .sensor_idx  = state->sensor_idx,
        .sensor_type = state->sensor_type
    };

    demo_get_sensor_config(this, instance, &chosen_sample_rate, &chosen_report_rate, &sensor_client_present);
    DEMO_SENSOR_LOG(HIGH, this, "sensor %d sample_rate = %d, report_rate = %d, state->chosen_sample_rate = %d", state->sensor_type,
                    (uint32_t)chosen_sample_rate, (uint32_t)chosen_report_rate, (uint32_t)state->chosen_sample_rate);
    if (chosen_sample_rate != state->chosen_sample_rate) {
        sample_rate_need_change = true;
        state->chosen_sample_rate = chosen_report_rate;
    }
    //TODO: need double check whether update or not
    for (demo_sensor_idx i = ACC; i < NUMS; i++) {
        if (state->sensor_type == (1 << i)) {
            istate->ss_cfgs[i].sample_rate = chosen_sample_rate;
            istate->ss_cfgs[i].report_rate = chosen_report_rate;
            if (!IS_ZERO(istate->ss_cfgs[i].sample_rate)) {
                // istate->ss_cfgs[i].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[i].sample_rate);
                istate->ss_cfgs[i].sample_intvl_req = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[i].sample_rate);
            } else {
                // istate->ss_cfgs[i].sample_intvl = 0;
                istate->ss_cfgs[i].sample_intvl_req = 0;
            }
        }
    }


#if 0
    if (state->sensor_type == DEMO_ACCEL) {
        istate->ss_cfgs[ACC].sample_rate = chosen_sample_rate;
        istate->ss_cfgs[ACC].report_rate = chosen_report_rate;
        if (!IS_ZERO(istate->ss_cfgs[ACC].sample_rate)) {
            istate->ss_cfgs[ACC].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[ACC].sample_rate);
        } else {
            istate->ss_cfgs[ACC].sample_intvl = 0;
        }
    }

    if (state->sensor_type == DEMO_GYRO) {
        istate->ss_cfgs[GYR].sample_rate = chosen_sample_rate;
        istate->ss_cfgs[GYR].report_rate = chosen_report_rate;
        istate->ss_cfgs[GYR].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[GYR].sample_rate);
        if (!IS_ZERO(istate->ss_cfgs[GYR].sample_rate)) {
            istate->ss_cfgs[GYR].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[GYR].sample_rate);
        } else {
            istate->ss_cfgs[GYR].sample_intvl = 0;
        }
    }

    if (state->sensor_type == DEMO_TEMP) {
        istate->ss_cfgs[TEMP].sample_rate = chosen_sample_rate;
        istate->ss_cfgs[TEMP].report_rate = chosen_report_rate;
        istate->ss_cfgs[TEMP].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[TEMP].sample_rate);
        if (!IS_ZERO(istate->ss_cfgs[TEMP].sample_rate)) {
            istate->ss_cfgs[TEMP].sample_intvl = sns_convert_ns_to_ticks(1e9 / istate->ss_cfgs[TEMP].sample_rate);
        } else {
            istate->ss_cfgs[TEMP].sample_intvl = 0;
        }
    }
#endif
    req_payload.sample_rate = (uint32_t)chosen_sample_rate;
    req_payload.report_rate = (uint32_t)chosen_report_rate;
    demo_set_client_config(this, instance, req_payload, SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_CONFIG);
}

/* update this attr once sensor available */
void demo_publish_available(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    sns_std_attr_value_data value = SNS_ATTR;
    value.has_boolean = true;
    value.boolean = true;
    sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_AVAILABLE, &value, 1, true);
    DEMO_SENSOR_LOG(HIGH, this, "sensor %d available now", state->sensor_type);
}

static sns_rc demo_register_com_port(sns_sensor *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    demo_state *state = (demo_state *)this->state->state;

    if (!state->scp_service) {
        DEMO_SENSOR_LOG(HIGH, this, "invalid scp_service!");
        ret = SNS_RC_FAILED;
        return ret;
    }

    DEMO_SENSOR_LOG(HIGH, this, "[%d, %d, %d, %d, 0x%02x], port_handle = 0x%p",
        state->common->com_port.com_config.bus_instance, state->common->com_port.com_config.bus_type,
        state->common->com_port.com_config.min_bus_speed_KHz,
        state->common->com_port.com_config.max_bus_speed_KHz,
        state->common->com_port.com_config.slave_control,
        state->common->com_port.port_handle);

    /* register and open com port */
    if (!state->common->com_port.port_handle) {
        ret = state->scp_service->api->sns_scp_register_com_port(&state->common->com_port.com_config,
                                                                 &state->common->com_port.port_handle);
        if (ret == SNS_RC_SUCCESS && state->common->com_port.port_handle != NULL) {
            /* open com port */
            ret = state->scp_service->api->sns_scp_open(state->common->com_port.port_handle);
            if (!ret) {
                DEMO_SENSOR_LOG(HIGH, this, "open com port ok");
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "failed com port !");
            }
        } else {
            DEMO_SENSOR_LOG(HIGH, this, "failed to register com port, ret = %d, port null?->0x%p!",
                ret, state->common->com_port.port_handle);
        }
        // no use here because power in off status
        // demo_switch_bus(state->common->com_port.com_config.bus_type, state->scp_service,
        //                 &state->common->com_port.port_handle);
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "port_handle already ok???");
    }

    return ret;
}


static sns_rc demo_register_power_rail(sns_sensor *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    demo_state *state = (demo_state *)this->state->state;
    sns_service_manager *sm = this->cb->get_service_manager(this);

    DEMO_SENSOR_LOG(HIGH, this, "%d, %d, %d, %d",
                    state->common->rail_cfg.num_of_rails, state->common->rail_cfg.rail_vote,
                    strcmp(state->common->rail_cfg.rails[0].name, "/pmic/client/sensor_vddio"),
                    strcmp(state->common->rail_cfg.rails[1].name, "/pmic/client/sensor_vdd"));

    state->common->rail_cfg.rail_vote = SNS_RAIL_OFF;
    //state->common->rail_cfg.rail_vote = state->common->registry_rail_on_state;
    if (!state->pwr_rail_service) {
        state->pwr_rail_service = (sns_pwr_rail_service *)sm->get_service(sm, SNS_POWER_RAIL_SERVICE);
    }

    if (state->pwr_rail_service) {
        ret = state->pwr_rail_service->api->sns_register_power_rails(state->pwr_rail_service,
                                                                     &state->common->rail_cfg);
        if (ret) {
            DEMO_SENSOR_LOG(HIGH, this, "register power rail failed, ret = %d!", ret);
        }
    }

    return ret;
}

/*
 * actually, this func is not related with power rail, just start a timer
 * it can be called after demo_sensor_update_power_rail_vote, this func will control power
 * then use this timer to handle later works according to power_state
 */
void demo_sensor_start_power_rail_timer(sns_sensor *const this, sns_time timeout_ticks,
                                        demo_power_rail_pending_state pwrail_pend_state)
{
    size_t len = 0;
    uint8_t buffer[32] = {0};
    sns_rc ret = SNS_RC_SUCCESS;
    demo_state *state = (demo_state *)this->state->state;

    sns_timer_sensor_config timer_cfg = sns_timer_sensor_config_init_default;
    timer_cfg.is_periodic = false;
    timer_cfg.start_time = sns_get_system_time();
    timer_cfg.timeout_period = timeout_ticks;

    if (!state->timer_stream) {
        sns_service_manager *sm = this->cb->get_service_manager(this);
        sns_stream_service *stream_svc = (sns_stream_service *)sm->get_service(sm, SNS_STREAM_SERVICE);
        ret = stream_svc->api->create_sensor_stream(stream_svc, this, state->timer_suid, &state->timer_stream);
        if (ret) {
            DEMO_SENSOR_LOG(HIGH, this, "failed to create timer stream");
            //return ret;
        } else {
            DEMO_SENSOR_LOG(HIGH, this, "create timer stream ok");
        }
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "already timer stream");
    }

    len = pb_encode_request(buffer, sizeof(buffer), &timer_cfg, sns_timer_sensor_config_fields, NULL);
    if (len > 0 && state->timer_stream) {
        sns_request timer_req = {
            .message_id = SNS_TIMER_MSGID_SNS_TIMER_SENSOR_CONFIG,
            .request = buffer,
            .request_len = len
        };
        ret = state->timer_stream->api->send_request(state->timer_stream, &timer_req);
        if (!ret) {
            state->pwrail_pending_state = pwrail_pend_state;
        }
        DEMO_SENSOR_LOG(HIGH, this, "send timer request, ret = %d", ret);
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "requested encode error");
    }
}

sns_rc demo_start_hw_detect(sns_sensor *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    uint8_t chip_id[2] = {0};
    demo_state *state = (demo_state *)this->state->state;
    sns_time timeout_ticks = sns_convert_ns_to_ticks(100 * 1000 * 1000);  // 100ms

    if (!state->scp_service) {
        DEMO_SENSOR_LOG(HIGH, this, "invalid scp service!");
    }
    if (!state->common->com_port.port_handle) {
        demo_sensor_register_pf_resource(this);
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "re-alloc port_handle!");
    }

    state->common->rail_cfg.rail_vote = state->common->registry_rail_on_state;
    // DEMO_SENSOR_LOG(HIGH, this, "sensor %d vote %d -> %d",
                    // state->sensor_type, state->common->rail_cfg.rail_vote, SNS_RAIL_ON_LPM);// -> ???
    demo_sensor_update_power_rail_vote(this, state->common->rail_cfg.rail_vote, NULL);
    demo_sensor_start_power_rail_timer(this, timeout_ticks, DEMO_POWER_RAIL_PENDING_INIT);

    demo_switch_bus(state->common->com_port.com_config.bus_type, state->scp_service,
                    &state->common->com_port.port_handle);

    demo_read_chip_id(state->scp_service, &state->common->com_port.port_handle, chip_id);
    if (chip_id[0] == DEMO_CHIP_ID_0 || chip_id[0] == DEMO_CHIP_ID_1) {
        state->hw_is_present = true;
    } else {
        state->hw_is_present = false;
        ret = SNS_RC_INVALID_LIBRARY_STATE;
    }
    DEMO_SENSOR_LOG(HIGH, this, "get dummy i2c:%d, i3c:%d, spi:%d", state->common->i2c_dummy,
                    state->common->i3c_dummy, state->common->spi_dummy);
    DEMO_SENSOR_LOG(HIGH, this, "sensor %d read chip id = [0x%02x, 0x%02x]", state->sensor_type,
                    chip_id[0], chip_id[1]);
    /* bus power down and close com port */
    if (state->common->com_port.port_handle) {
        state->scp_service->api->sns_scp_update_bus_power(state->common->com_port.port_handle, false);
        state->scp_service->api->sns_scp_close(state->common->com_port.port_handle);
        state->scp_service->api->sns_scp_deregister_com_port(&state->common->com_port.port_handle);
        state->common->com_port.port_handle = NULL;
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "port handle NULL");
    }
    /* sensor power down */
    demo_sensor_update_power_rail_vote(this, SNS_RAIL_OFF, NULL);

    // if(state->hw_is_present) {
    //     demo_publish_available(this);
    // }

    state->hw_detection_finished = true;

    return ret;
}

void demo_update_registry_attributes(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    {
        sns_std_attr_value_data value = {
            .has_boolean = true,
            .boolean = state->ss_cfg.is_dri
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_DRI, &value, 1, false);
    }

    {
        sns_std_attr_value_data value = {
            .has_boolean = true,
            .boolean = state->ss_cfg.sync_stream
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_STREAM_SYNC, &value, 1, false);
    }

    {
        sns_std_attr_value_data value = {
            .has_sint = true,
            .sint = state->hw_idx
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_HW_ID, &value, 1, false);
    }

    if (state->sensor_type == DEMO_ACCEL || state->sensor_type == DEMO_GYRO ||
        state->sensor_type == DEMO_TEMP) {
        sns_std_attr_value_data value = SNS_ATTR;
        value.has_flt = true;
        value.flt = (state->sensor_type == DEMO_ACCEL)
                        // ? demo_accel_res[state->ss_cfg.res_idx] * ACC_RSL_CONVERSION
                        // : demo_gyro_res[state->ss_cfg.res_idx] * GYRO_CONVERSION;
                        ? demo_ss_rsl[ACC][state->ss_cfg.res_idx] * ACC_RSL_CONVERSION
                        : demo_ss_rsl[GYR][state->ss_cfg.res_idx] * GYRO_CONVERSION;
        if (state->sensor_type == DEMO_TEMP) {
            // value.flt = demo_temp_res[state->ss_cfg.res_idx];
            value.flt = demo_ss_rsl[TEMP][state->ss_cfg.res_idx];
        }
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_SELECTED_RESOLUTION, &value, 1, false);
    }

    if (state->sensor_type == DEMO_ACCEL || state->sensor_type == DEMO_GYRO ||
        state->sensor_type == DEMO_TEMP) {
        sns_std_attr_value_data values[] = {SNS_ATTR};
        sns_std_attr_value_data range_min_max[] = {SNS_ATTR, SNS_ATTR};
        range_min_max[0].has_flt = true;
        range_min_max[0].flt = (state->sensor_type == DEMO_ACCEL)
                                //    ? demo_accel_ranges[state->ss_cfg.res_idx].min * ACC_CONVERSION
                                //    : demo_gyro_ranges[state->ss_cfg.res_idx].min * GYRO_CONVERSION;
                                   ? demo_ss_ranges[ACC][state->ss_cfg.res_idx].min * ACC_CONVERSION
                                   : demo_ss_ranges[GYR][state->ss_cfg.res_idx].min * GYRO_CONVERSION;
        range_min_max[1].has_flt = true;
        range_min_max[1].flt = (state->sensor_type == DEMO_ACCEL)
                                //    ? demo_accel_ranges[state->ss_cfg.res_idx].max * ACC_CONVERSION
                                //    : demo_gyro_ranges[state->ss_cfg.res_idx].max * GYRO_CONVERSION;
                                   ? demo_ss_ranges[ACC][state->ss_cfg.res_idx].max * ACC_CONVERSION
                                   : demo_ss_ranges[GYR][state->ss_cfg.res_idx].max * GYRO_CONVERSION;
        if (state->sensor_type == DEMO_TEMP) {
            // range_min_max[0].flt = demo_temp_ranges[state->ss_cfg.res_idx].min;
            // range_min_max[1].flt = demo_temp_ranges[state->ss_cfg.res_idx].max;
            range_min_max[0].flt = demo_ss_ranges[TEMP][state->ss_cfg.res_idx].min;
            range_min_max[1].flt = demo_ss_ranges[TEMP][state->ss_cfg.res_idx].max;
        }
        values[0].has_subtype = true;
        values[0].subtype.values.funcs.encode = sns_pb_encode_attr_cb;
        values[0].subtype.values.arg =
            &((pb_buffer_arg){.buf = range_min_max, .buf_len = ARR_SIZE(range_min_max)});
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_SELECTED_RANGE, &values[0], ARR_SIZE(values), true);
    }

    DEMO_SENSOR_LOG(HIGH, this, "sensor %d upate attributes", state->sensor_type);
}


/* it is different with linux/other OS, register/deregister is very frequently in SEE */
void demo_sensor_register_pf_resource(sns_sensor *const this)
{
    /* register com port */
    demo_register_com_port(this);

    /* register irq resource */
    /* no need because SEE will un/register irq */

    /* register power rail */
    demo_register_power_rail(this);
}

void demo_sensor_parse_registry(sns_sensor *const this, sns_sensor_event *event)
{
    demo_state *state = (demo_state *)this->state->state;
    pb_istream_t stream = pb_istream_from_buffer((void *)event->event, event->event_len);

    if (!event) {
        DEMO_SENSOR_LOG(HIGH, this, "invalid event!");
        return;
    }

    DEMO_SENSOR_LOG(HIGH, this, "sensor %d, msg_id = %d", state->sensor_type, event->message_id);

    if (event->message_id == SNS_REGISTRY_MSGID_SNS_REGISTRY_READ_EVENT) {
        sns_registry_read_event read_event = sns_registry_read_event_init_default;
        pb_buffer_arg group_name = {0, 0};
        read_event.name.arg = &group_name;
        read_event.name.funcs.decode = pb_decode_string_cb;
        if (!pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
            DEMO_SENSOR_LOG(HIGH, this, "failed to decode registry event");
        } else {
            stream = pb_istream_from_buffer((void *)event->event, event->event_len);
            /* sensor config */
            if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_CONFIG_ACCEL], group_name.buf_len) ||
                !strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_CONFIG_GYRO], group_name.buf_len) ||
                !strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_CONFIG_TEMP], group_name.buf_len) ||
                !strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_CONFIG_MD], group_name.buf_len)) {  //||
                DEMO_SENSOR_LOG(HIGH, this, "parse_phy_sensor_cfg for acc/gyro/temp/md");
                {
                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 1,
                        .parse_info[0] = {
                            .group_name = "config",
                            .parse_func = sns_registry_parse_phy_sensor_cfg,
                            .parsed_buffer = &state->ss_cfg
                        }
                    };
                    read_event.data.items.funcs.decode = sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;
                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->registry_ss_cfg_received = true;
                        if (state->sensor_type == DEMO_ACCEL) {
                            // state->common->acc_ss_cfg = state->ss_cfg;
                            DEMO_SENSOR_LOG(HIGH, this, "sensor acc received cfg");
                        } else if (state->sensor_type == DEMO_GYRO) {
                            // state->common->gyr_ss_cfg = state->ss_cfg;
                            DEMO_SENSOR_LOG(HIGH, this, "sensor gyro received cfg");
                        } else if (state->sensor_type == DEMO_TEMP) {
                            // state->common->tmp_ss_cfg = state->ss_cfg;
                            DEMO_SENSOR_LOG(HIGH, this, "sensor temp received cfg");
                        } else if (state->sensor_type == DEMO_MOTION) {
                            // state->common->md_ss_cfg = state->ss_cfg;
                            DEMO_SENSOR_LOG(HIGH, this, "sensor motion received cfg");
                        }
                        DEMO_SENSOR_LOG(HIGH, this, "sensor %d get [is_dri, hw_id, res_idx, sync_stream]->[%d, %d, %d, %d]",
                                        state->sensor_type, state->ss_cfg.is_dri, (uint8_t)state->ss_cfg.hw_id,
                                        state->ss_cfg.res_idx, state->ss_cfg.sync_stream);
                        demo_update_registry_attributes(this);
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "parse_phy_sensor_cfg get nothing!");
                    }
                }
                /* platform config */
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_CONFIG], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse_phy_sensor_pf_cfg for platform");
                {
                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 1,
                        .parse_info[0] = {
                            .group_name = "config",
                            .parse_func = sns_registry_parse_phy_sensor_pf_cfg,
                            .parsed_buffer = &state->common->pf_cfg
                        }
                    };
                    read_event.data.items.funcs.decode = sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;
                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->common->registry_pf_cfg_received = true;
                        /* com port config */
                        state->common->com_port.com_config.bus_type = state->common->pf_cfg.bus_type;
                        state->common->com_port.com_config.bus_instance = state->common->pf_cfg.bus_instance;
                        state->common->com_port.com_config.slave_control = state->common->pf_cfg.slave_config;  // i2c addr
                        state->common->com_port.i2c_addr = state->common->pf_cfg.slave_config;                  // i2c addr
                        state->common->com_port.i3c_addr = state->common->pf_cfg.i3c_address;                  // i3c addr
                        state->common->com_port.com_config.min_bus_speed_KHz = state->common->pf_cfg.min_bus_speed_khz;
                        state->common->com_port.com_config.max_bus_speed_KHz = state->common->pf_cfg.max_bus_speed_khz;
                        state->common->com_port.com_config.reg_addr_type = state->common->pf_cfg.reg_addr_type;
                        if (state->common->com_port.com_config.bus_type == SNS_BUS_I3C) {
                            state->common->com_port.dummy_byte = state->common->i3c_dummy;
                            state->common->com_port.com_config.slave_control = state->common->pf_cfg.i3c_address;  // i3c addr
                        } else if (state->common->com_port.com_config.bus_type == SNS_BUS_I2C) {
                            state->common->com_port.dummy_byte = state->common->i2c_dummy;
                        } else if (state->common->com_port.com_config.bus_type == SNS_BUS_SPI) {
                            state->common->com_port.dummy_byte = state->common->spi_dummy;
                        } else {
                            state->common->com_port.dummy_byte = 0;
                            DEMO_SENSOR_LOG(HIGH, this, "unknown bus type!");
                        }
                        DEMO_SENSOR_LOG(HIGH, this, "min_odr = %d, max_odr = %d",
                                        state->common->pf_cfg.min_odr,
                                        state->common->pf_cfg.max_odr);
                        /* power rail config */
                        state->common->rail_cfg.num_of_rails = state->common->pf_cfg.num_rail;
                        state->common->registry_rail_on_state = state->common->pf_cfg.rail_on_state;
                        sns_strlcpy(state->common->rail_cfg.rails[0].name,
                                    state->common->pf_cfg.vddio_rail,
                                    sizeof(state->common->rail_cfg.rails[0].name));
                        sns_strlcpy(state->common->rail_cfg.rails[1].name,
                                    state->common->pf_cfg.vdd_rail,
                                    sizeof(state->common->rail_cfg.rails[1].name));

                        DEMO_SENSOR_LOG(HIGH, this, "bus_type = %d, bus_instance = %d, slave_cfg = 0x%04x",
                                        state->common->pf_cfg.bus_type, state->common->pf_cfg.bus_instance,
                                        state->common->pf_cfg.slave_config);
                        DEMO_SENSOR_LOG(HIGH, this, "min_bus_speed_kHz= %d, max_bus_speed_kHz = %d, reg_addr_type = %d",
                            state->common->pf_cfg.min_bus_speed_khz, state->common->pf_cfg.max_bus_speed_khz,
                            state->common->pf_cfg.reg_addr_type);

                        state->common->irq_info.irq_config.interrupt_num = state->common->pf_cfg.dri_irq_num;
                        state->common->irq_info.irq_config.interrupt_pull_type = state->common->pf_cfg.irq_pull_type;
                        state->common->irq_info.irq_config.interrupt_drive_strength = state->common->pf_cfg.irq_drive_strength;
                        state->common->irq_info.irq_config.interrupt_trigger_type = state->common->pf_cfg.irq_trigger_type;
                        state->common->irq_info.irq_config.is_chip_pin = state->common->pf_cfg.irq_is_chip_pin;
                        DEMO_SENSOR_LOG(HIGH, this, "irq_num = %d, irq_pull_type = %d, is_chip_pin = %d",
                                        state->common->pf_cfg.dri_irq_num, state->common->pf_cfg.irq_pull_type,
                                        state->common->pf_cfg.irq_is_chip_pin);
                        DEMO_SENSOR_LOG(HIGH, this, "irq_drive_strength = %d, irq_trigger_type = %d, rigid_body_type = %d",
                            state->common->pf_cfg.irq_drive_strength,
                            state->common->pf_cfg.irq_trigger_type,
                            state->common->pf_cfg.rigid_body_type);

                        demo_sensor_register_pf_resource(this);
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "parse_phy_sensor_pf_cfg get nothing!");
                    }
                }
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_CONFIG_MD], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse_md_cfg for motion detect");
                {
                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 1,
                        .parse_info[0] = {
                            .group_name = "config",
                            .parse_func = sns_registry_parse_md_cfg,
                            .parsed_buffer = &state->common->md_cfg
                        }
                    };
                    read_event.data.items.funcs.decode = sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;
                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->common->registry_md_cfg_received = true;
                        DEMO_SENSOR_LOG(HIGH, this, "common->md_cfg [%d, %d, %d]",
                                        state->common->md_cfg.disable,
                                        (int32_t)(state->common->md_cfg.thresh * 1000000),
                                        (int32_t)(state->common->md_cfg.win * 1000000));
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "parse_md_cfg get nothing!");
                    }
                }
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_PLACEMENT], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse placement");
                {
                    uint8_t arr_idx = 0;
                    pb_float_arr_arg arr_arg = {
                        .arr = state->common->placement,//array[12]
                        .arr_index = &arr_idx,
                        .arr_len = 12
                    };

                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 1,
                        .parse_info[0] = {
                            .group_name = "placement",
                            .parse_func = sns_registry_parse_float_arr,
                            .parsed_buffer = &arr_arg,
                        }
                    };
                    read_event.data.items.funcs.decode = sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;
                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->common->registry_placement_received = true;
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "parse placement get nothing!");
                    }
                }
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_ORIENT], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse orient");
                {
                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 1,
                        .parse_info[0] = {
                            .group_name = "orient",
                            .parse_func = sns_registry_parse_axis_orientation,
                            .parsed_buffer = state->common->axis_map
                        }
                    };

                    read_event.data.items.funcs.decode = &sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;
                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->common->registry_orient_received = true;
                        DEMO_SENSOR_LOG(HIGH, this, "Input Axis %d maps to Output Axis %d with inversion %d",
                            state->common->axis_map[0].ipaxis, state->common->axis_map[0].opaxis,
                            state->common->axis_map[0].invert);

                        DEMO_SENSOR_LOG(HIGH, this, "Input Axis %d maps to Output Axis %d with inversion %d",
                            state->common->axis_map[1].ipaxis, state->common->axis_map[1].opaxis,
                            state->common->axis_map[1].invert);

                        DEMO_SENSOR_LOG(HIGH, this, "Input Axis %d maps to Output Axis %d with inversion %d",
                            state->common->axis_map[2].ipaxis, state->common->axis_map[2].opaxis,
                            state->common->axis_map[2].invert);
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "parse orient get nothing!");
                    }
                }
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_ACCEL], group_name.buf_len) ||
                !strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_GYRO], group_name.buf_len) ||
                !strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_TEMP], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse acc/gyro/temp fac_cal");

                float fac_cal_bias[TRIAXIS_NUM];
                {
                    uint8_t bias_arr_index = 0, scale_arr_index = 0;
                    pb_float_arr_arg bias_arr_arg = {
                        .arr = fac_cal_bias,
                        .arr_index = &bias_arr_index,
                        .arr_len = TRIAXIS_NUM
                    };

                    pb_float_arr_arg scale_arr_arg = {
                        .arr = state->fac_cal_scale,
                        .arr_index = &scale_arr_index,
                        .arr_len = TRIAXIS_NUM
                    };

                    sns_registry_decode_arg arg = {
                        .item_group_name = &group_name,
                        .parse_info_len = 3,
                        .parse_info[0] = {
                            .group_name = "bias",
                            .parse_func = sns_registry_parse_float_arr,
                            .parsed_buffer = &bias_arr_arg
                        },
                        .parse_info[1] = {
                            .group_name = "scale",
                            .parse_func = sns_registry_parse_float_arr,
                            .parsed_buffer = &scale_arr_arg
                        },
                        .parse_info[2] = {
                            .group_name = "corr_mat",
                            .parse_func = sns_registry_parse_corr_matrix_3,
                            .parsed_buffer = &state->fac_cal_corr_mat
                        }
                    };

                    read_event.data.items.funcs.decode = &sns_registry_item_decode_cb;
                    read_event.data.items.arg = &arg;

                    if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->registry_fac_cal_received = true;
                        state->fac_cal_version = arg.version;
                        DEMO_SENSOR_LOG(HIGH, this, "sensor %d received fac_cal", state->sensor_type);

                        //uint8_t i;
                        for (uint8_t i = 0; i < TRIAXIS_NUM; i++) {
                            state->fac_cal_bias[i] = roundf(fac_cal_bias[i] * state->scale_factor);
                        }

                        if (state->fac_cal_scale[0] != 0.0f) {
                            state->fac_cal_corr_mat.e00 = state->fac_cal_scale[0];
                            state->fac_cal_corr_mat.e11 = state->fac_cal_scale[1];
                            state->fac_cal_corr_mat.e22 = state->fac_cal_scale[2];
                        }
                    } else {
                        DEMO_SENSOR_LOG(ERROR, this, "fac_cal error %d", state->sensor_type);
                    }
                }
            } else if (!strncmp((char *)group_name.buf, demo_registry_cfg[state->hw_idx][REG_CONFIG_SPEC], group_name.buf_len)) {
                DEMO_SENSOR_LOG(HIGH, this, "parse spec/.config");
                float data[3];
                uint8_t arr_idx = 0;
                pb_float_arr_arg arr_arg = {
                    .arr = data,
                    .arr_index = &arr_idx,
                    .arr_len = 3,
                };

                sns_registry_decode_arg arg = {
                    .item_group_name = &group_name,
                    .parse_info_len = 1,
                    .parse_info[0] = {
                        .group_name = "dummy_byte",
                        .parse_func = sns_registry_parse_float_arr,
                        .parsed_buffer = &arr_arg,
                    }
                };
                read_event.data.items.funcs.decode = sns_registry_item_decode_cb;
                read_event.data.items.arg = &arg;
                if (pb_decode(&stream, sns_registry_read_event_fields, &read_event)) {
                        state->common->i2c_dummy = (uint8_t)data[0];
                        state->common->i3c_dummy = (uint8_t)data[1];
                        state->common->spi_dummy = (uint8_t)data[2];
                        DEMO_SENSOR_LOG(
                            HIGH, this, "get dummy cfg [%d, %d, %d]", state->common->i2c_dummy,
                            state->common->i3c_dummy, state->common->spi_dummy);
                } else {
                    DEMO_SENSOR_LOG(HIGH, this, "parse spec.config get nothing!");
                }
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "unknown registry property, please check!");
            }
            state->common->registry_req_cnt--;
        }
    }
    DEMO_SENSOR_LOG(HIGH, this, "registry left %d events", state->common->registry_req_cnt);
}

static void send_registry_request(sns_sensor *const this, char *property)
{
    sns_rc ret = SNS_RC_SUCCESS;
    int8_t buffer[128] = {0};
    uint32_t encoded_len = 0;
    demo_state *state = (demo_state *)this->state->state;

    sns_registry_read_req read_request;
    pb_buffer_arg data = (pb_buffer_arg){.buf = property, .buf_len = (strlen(property) + 1)};
    read_request.name.arg = &data;
    read_request.name.funcs.encode = pb_encode_string_cb;

    encoded_len = pb_encode_request(buffer, sizeof(buffer), &read_request,
                                    sns_registry_read_req_fields, NULL);
    if (encoded_len > 0) {
        sns_request request = (sns_request) {
            .request_len = encoded_len,
            .request = buffer,
            .message_id = SNS_REGISTRY_MSGID_SNS_REGISTRY_READ_REQ
        };
        ret = state->registry_stream->api->send_request(state->registry_stream, &request);
        DEMO_SENSOR_LOG(HIGH, this, "sensor %d send req, ret = %d",
                        state->sensor_type, ret);
        state->common->registry_req_cnt++;
    }
}

void demo_sensor_send_registry_request(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    sns_service_manager *sm = this->cb->get_service_manager(this);
    sns_stream_service *stream_svc = (sns_stream_service *)sm->get_service(sm, SNS_STREAM_SERVICE);
    DEMO_SENSOR_LOG(HIGH, this, "sensor %d registry_suid = %"PRIsuid, state->sensor_type,
                    SNS_PRI_SUID(&state->registry_suid));

    if (!state->registry_stream) {
        stream_svc->api->create_sensor_stream(stream_svc, this, state->registry_suid, &state->registry_stream);

        if (state->sensor_type == DEMO_ACCEL) {
            // send common registry req: only need ONCE!
            DEMO_SENSOR_LOG(HIGH, this, "send_request for registry of platform");
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_CONFIG_SPEC]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_CONFIG]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_PLACEMENT]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_ORIENT]);

            DEMO_SENSOR_LOG(HIGH, this, "send_request for registry of acc");
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_CONFIG_ACCEL]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_ACCEL]);
        }

        if (state->sensor_type == DEMO_GYRO) {
            DEMO_SENSOR_LOG(HIGH, this, "send_request for registry of gyro");
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_CONFIG_GYRO]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_GYRO]);
        }

        if (state->sensor_type == DEMO_MOTION) {
            DEMO_SENSOR_LOG(HIGH, this, "send_request for registry of motion");
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_CONFIG_MD]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_CONFIG_MD]);
        }

        if (state->sensor_type == DEMO_TEMP) {
            DEMO_SENSOR_LOG(HIGH, this, "send_request for registry of temp");
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_CONFIG_TEMP]);
            send_registry_request(this, demo_registry_cfg[state->hw_idx][REG_PLATFORM_FAC_CAL_TEMP]);
        }
    }
}

void demo_publish_default_registry_attributes(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;

    {//sensor name
        sns_std_attr_value_data value = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg){.buf = ss_name, .buf_len = sizeof(ss_name)})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_NAME, &value, 1, false);
    }

    {//sensor vendor
        sns_std_attr_value_data value = {
            .str.funcs.encode = pb_encode_string_cb,
            .str.arg = &((pb_buffer_arg){.buf = ss_vendor, .buf_len = sizeof(ss_vendor)})
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_VENDOR, &value, 1, false);
    }

    {//driver version
        sns_std_attr_value_data value = {.has_sint = true, .sint = DEMO_SEE_DD_ATTRIB_VERSION};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_VERSION, &value, 1, false);
    }

    {//TODO:fifo size, tbd 600/12=50frames
        sns_std_attr_value_data value = {.has_sint = true, .sint = 50};
        if (state->sensor_type == DEMO_MOTION) {
            value.sint = 0;
        }
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_FIFO_SIZE, &value, 1, false);
    }

    {//power mode supported
        sns_std_attr_value_data values[] = {
            {.str.funcs.encode = pb_encode_string_cb, .str.arg = &((pb_buffer_arg) {.buf = "LPM", .buf_len = sizeof("LPM")})},
            {.str.funcs.encode = pb_encode_string_cb, .str.arg = &((pb_buffer_arg) {.buf = "NORMAL", .buf_len = sizeof("NORMAL")})}
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_OP_MODES, values, ARR_SIZE(values), false);
    }

    {//event size
        sns_std_attr_value_data value = SNS_ATTR;
        if (state->sensor_type == DEMO_ACCEL || state->sensor_type == DEMO_GYRO) {
            float data[3] = {0};
            state->encoded_event_len = pb_get_encoded_size_sensor_stream_event(data, ARR_SIZE(data));
        } else if (state->sensor_type == DEMO_MOTION) {
            sns_motion_detect_event stream_event = sns_motion_detect_event_init_default;
            pb_get_encoded_size(&state->encoded_event_len, sns_motion_detect_event_fields,
                                &stream_event);
        } else if (state->sensor_type == DEMO_TEMP) {
            float data[1] = {0};
            state->encoded_event_len = pb_get_encoded_size_sensor_stream_event(data, ARR_SIZE(data));
        }
        value.has_sint = true;
        value.sint = state->encoded_event_len;
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_EVENT_SIZE, &value, 1, false);
    }

    {//stream type
        sns_std_attr_value_data value = SNS_ATTR;
        if (state->sensor_type == DEMO_ACCEL || state->sensor_type == DEMO_GYRO ||
            state->sensor_type == DEMO_TEMP) {
            value.has_sint = true;
            value.sint = SNS_STD_SENSOR_STREAM_TYPE_STREAMING;
        } else if (state->sensor_type == DEMO_MOTION) {
            value.has_sint = true;
            value.sint = SNS_STD_SENSOR_STREAM_TYPE_SINGLE_OUTPUT;
        }
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_STREAM_TYPE, &value, 1, false);
    }

    {// dynamic
        sns_std_attr_value_data value = {
            .has_boolean = true,
            .boolean = false
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_DYNAMIC, &value, 1, false);
    }

    {// rigid body type
        //const sns_std_sensor_rigid_body_type rigid_body = SNS_STD_SENSOR_RIGID_BODY_TYPE_DISPLAY;
        sns_std_attr_value_data value = {
            .has_sint = true,
            .sint = SNS_STD_SENSOR_RIGID_BODY_TYPE_DISPLAY
        };
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_RIGID_BODY, &value, 1, true);//fixed value?
    }

    {// placement
        if (state->sensor_type == DEMO_ACCEL || state->sensor_type == DEMO_GYRO) {
            sns_std_attr_value_data values[] = {SNS_ATTR, SNS_ATTR, SNS_ATTR, SNS_ATTR,
                                                SNS_ATTR, SNS_ATTR, SNS_ATTR, SNS_ATTR,
                                                SNS_ATTR, SNS_ATTR, SNS_ATTR, SNS_ATTR};
            for (uint8_t i = 0; i < ARR_SIZE(values); i++) {
                values[i].has_flt = true;
                values[i].flt = 0;
            }
            sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_PLACEMENT, values, ARR_SIZE(values), true);  // fixed?
        }
    }

    {  // hw id
        sns_std_attr_value_data value = {.has_sint = true, .sint = 0};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_HW_ID, &value, 1, false);
    }

    {// data ready irq
        sns_std_attr_value_data value = {.has_boolean = true, .boolean = true};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_DRI, &value, 1, false);
    }

    {// physical sensor
        sns_std_attr_value_data value = {.has_boolean = true, .boolean = true};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_PHYSICAL_SENSOR, &value, 1, false);
    }

    {// stream sync
        sns_std_attr_value_data value = {.has_boolean = true, .boolean = false};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_STREAM_SYNC, &value, 1, false);
    }

    {  // available now?
        sns_std_attr_value_data value = {.has_boolean = true, .boolean = false};
        sns_publish_attribute(this, SNS_STD_SENSOR_ATTRID_AVAILABLE, &value, 1, false);
    }
}

//#define LOOKUP_CB
#ifdef LOOKUP_CB
/* 1. once lookup_get called, this cb will called before lookup_get finished
 * so you cannot use state->xxx_suid before lookup_get finished
 * 2. once cb called, event will send to cb, others cannot recv events even if cb does't handle it...
 */
bool demo_suid_lookup_cb(sns_sensor *const this, char const *data_type, sns_sensor_event *event)
{
    demo_state *state = (demo_state *)this->state->state;

    UNUSED_VAR(state);
    UNUSED_VAR(event);

    if (!strcmp(data_type, "registry")) {
        DEMO_SENSOR_LOG(HIGH, this, "registry event");
        //demo_sensor_send_registry_request(this);
    } else if (!strcmp(data_type, "timer")) {
        DEMO_SENSOR_LOG(HIGH, this, "timer event");
    } else if (!strcmp(data_type, "interrupt")) {
        DEMO_SENSOR_LOG(HIGH, this, "irq event");
    } else if (!strcmp(data_type, "async_com_port")) {
        DEMO_SENSOR_LOG(HIGH, this, "scp event");
    } else {
        DEMO_SENSOR_LOG(HIGH, this, " unknown event");
    }
    return true;
}
#endif

void demo_common_init(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    struct sns_service_manager *sm = this->cb->get_service_manager(this);
    state->diag_service = (sns_diag_service *)sm->get_service(sm, SNS_DIAG_SERVICE);
    state->scp_service = (sns_sync_com_port_service *)sm->get_service(sm, SNS_SYNC_COM_PORT_SERVICE);
    state->pwr_rail_service = (sns_pwr_rail_service *)sm->get_service(sm, SNS_POWER_RAIL_SERVICE);

    if (!state->diag_service || !state->scp_service) {
        DEMO_SENSOR_LOG(HIGH, this, "failed to get diag or scp service!");
        return;
    }


    state->common = &g_common[state->hw_idx];
    switch (state->sensor_type) {
    case DEMO_ACCEL:
        state->common->acc_state = state;
        state->common->states[ACC] = state;
        break;
    case DEMO_GYRO:
        state->common->gyr_state = state;
        state->common->states[GYR] = state;
        break;
    case DEMO_MOTION:
        state->common->md_state = state;
        state->common->states[MOTION] = state;
        break;
    case DEMO_TEMP:
        state->common->temp_state = state;
        state->common->states[TEMP] = state;
        break;
    default:
        DEMO_SENSOR_LOG(HIGH, this, "unknown sensor type = %d!", state->sensor_type);
        break;
    }

    DEMO_SENSOR_LOG(HIGH, this, "sensor %d, state = 0x%p", state->sensor_type, state);

    SNS_SUID_LOOKUP_INIT(state->suid_lookup_data, NULL);
    sns_suid_lookup_add(this, &state->suid_lookup_data, "timer");
    sns_suid_lookup_add(this, &state->suid_lookup_data, "interrupt");
    sns_suid_lookup_add(this, &state->suid_lookup_data, "async_com_port");
    sns_suid_lookup_add(this, &state->suid_lookup_data, "registry");


}