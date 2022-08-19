#include "sns_demo_sensor.h"

static sns_sensor_uid const *demo_get_sensor_uid(sns_sensor const *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    DEMO_SENSOR_LOG(HIGH, this, "hw_idx = %d, sensor type = %d, suid = %"PRIsuid,
                    state->hw_idx, state->sensor_type, SNS_PRI_SUID(&state->my_suid));
    return &state->my_suid;
}

//static void demo_sensor_exit_island(sns_sensor *const this)
void demo_sensor_exit_island(sns_sensor *const this)
{
    // force enable here
    sns_service_manager *sm = this->cb->get_service_manager(this);
    sns_island_service *island_svc = (sns_island_service *)sm->get_service(sm, SNS_ISLAND_SERVICE);
    island_svc->api->sensor_island_exit(island_svc, this);
    DEMO_SENSOR_LOG(HIGH, this, "demo_sensor_exit_island");
}

void demo_update_sibling_sensors(sns_sensor *const this)
{
    sns_sensor *lib_sensor = NULL;
    demo_state *lib_state = NULL;
    demo_state *state = (demo_state *)this->state->state;

    for (lib_sensor = this->cb->get_library_sensor(this, true); lib_sensor;
         lib_sensor = this->cb->get_library_sensor(this, false)) {
        lib_state = (demo_state *)lib_sensor->state->state;
        if (lib_state->sensor_type != state->sensor_type) {
        }

        demo_publish_available(lib_sensor);
    }
}

void demo_set_client_config(sns_sensor *const this, sns_sensor_instance *instance,
                            demo_req_payload req_payload, uint32_t message_id)
{
    DEMO_SENSOR_LOG(HIGH, this, "demo_set_client_config");
    sns_request client_request = {
        .request = &req_payload,
        .message_id = message_id,
        .request_len = sizeof(req_payload)
    };

    /* this func will call instance_api->set_client_config */
    this->instance_api->set_client_config(instance, &client_request);
}

void demo_sensor_process_suid_event(sns_sensor *const this)
{
    demo_state *state = (demo_state *)this->state->state;
    /*
     * if (!state->common->suid_lookup_data.suid_stream ||
     *     !state->common->suid_lookup_data.suid_stream->api->get_input_cnt(
     *         state->common->suid_lookup_data.suid_stream)) {
     *     return;
     * }
     */
    if (!state->suid_lookup_data.suid_stream) {
        return;
    }

    if (!sns_suid_lookup_complete(&state->suid_lookup_data)) {
        demo_sensor_exit_island(this);
        sns_suid_lookup_handle(this, &state->suid_lookup_data);

        if (SUID_IS_NULL(&state->registry_suid)) {
            if (sns_suid_lookup_get(&state->suid_lookup_data, "registry", &state->registry_suid)) {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d get registry suid = %"PRIsuid, state->sensor_type,
                                SNS_PRI_SUID(&state->registry_suid));
                // send registry req
                demo_sensor_exit_island(this);
                demo_sensor_send_registry_request(this);
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d cannot get registry suid", state->sensor_type);
            }
        }

        if (SUID_IS_NULL(&state->timer_suid)) {
            if (sns_suid_lookup_get(&state->suid_lookup_data, "timer", &state->timer_suid)) {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d get timer suid = %"PRIsuid, state->sensor_type,
                                SNS_PRI_SUID(&state->timer_suid));
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d cannot get timer suid", state->sensor_type);
            }
        }

        if (SUID_IS_NULL(&state->irq_suid)) {
            if (sns_suid_lookup_get(&state->suid_lookup_data, "interrupt", &state->irq_suid)) {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d get irq suid = %"PRIsuid, state->sensor_type,
                                SNS_PRI_SUID(&state->irq_suid));
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d cannot get irq suid", state->sensor_type);
            }
        }

        if (SUID_IS_NULL(&state->ascp_suid)) {
            if (sns_suid_lookup_get(&state->suid_lookup_data, "async_com_port", &state->ascp_suid)) {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d get async_com_port suid = %"PRIsuid, state->sensor_type,
                                SNS_PRI_SUID(&state->ascp_suid));
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "sensor %d cannot get async_com_port suid", state->sensor_type);
            }
        }

        if (sns_suid_lookup_complete(&state->suid_lookup_data)) {
            sns_suid_lookup_deinit(this, &state->suid_lookup_data);
            DEMO_SENSOR_LOG(HIGH, this, "sensor %d deinit lookup", state->sensor_type);
#ifdef LOOKUP_CB
            SNS_SUID_LOOKUP_INIT(state->common->suid_lookup_data, demo_suid_lookup_cb);
#else
            SNS_SUID_LOOKUP_INIT(state->common->suid_lookup_data, NULL);
#endif
            sns_memscpy(&state->common->suid_lookup_data, sizeof(state->common->suid_lookup_data),
                        &state->suid_lookup_data, sizeof(state->suid_lookup_data));
        }
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "sensor %d look up complete", state->sensor_type);
    }
}


static sns_rc demo_sensor_process_timer_event(sns_sensor *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    sns_sensor_event *event = NULL;
    demo_state *state = (demo_state *)this->state->state;

    if (!state->timer_stream ||
        !state->timer_stream->api->get_input_cnt(state->timer_stream)) {
        return ret;
    }

    // DEMO_SENSOR_LOG(HIGH, this, "cnt = %d", cnt);

    for (event = state->timer_stream->api->peek_input(state->timer_stream); event;
         event = state->timer_stream->api->get_next_input(state->timer_stream)) {
        sns_timer_sensor_event timer_event;
        pb_istream_t stream = pb_istream_from_buffer((pb_byte_t *)event->event, event->event_len);
        if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_EVENT) {
            if (pb_decode(&stream, sns_timer_sensor_event_fields, &timer_event)) {
                switch (state->pwrail_pending_state) {
                case DEMO_POWER_RAIL_PENDING_NONE:
                    DEMO_SENSOR_LOG(HIGH, this, "pwrail state = %d", state->pwrail_pending_state);
                    break;
                case DEMO_POWER_RAIL_PENDING_INIT:
                    DEMO_SENSOR_LOG(HIGH, this, "pwrail state = %d", state->pwrail_pending_state);
                    demo_sensor_exit_island(this);
                        // demo_start_hw_detect(this);// I suggested that only start_timer, and then read, not in process_timer_event
                        // state->hw_is_present = demo_read_chip_id(this);
                    if (state->hw_is_present) {
                        //demo_register_power_rail(this);
                        sns_sensor_util_remove_sensor_stream(this, &state->registry_stream);
                        demo_publish_available(this);
                        // demo_update_sibling_sensors(this);
                        DEMO_SENSOR_LOG(HIGH, this, "sensor %d init done", state->sensor_type);
                    } else {
                        ret = SNS_RC_INVALID_LIBRARY_STATE;
                        DEMO_SENSOR_LOG(HIGH, this, "all sensors of this library init failed!");
                    }
                    state->pwrail_pending_state = DEMO_POWER_RAIL_PENDING_NONE;
                    break;
                case DEMO_POWER_RAIL_PENDING_SET_CLIENT_REQ://for sensor instance
                    DEMO_SENSOR_LOG(HIGH, this, "pwrail state = %d", state->pwrail_pending_state);
                    sns_sensor_instance *instance = sns_sensor_util_get_shared_instance(this);
                    if (instance) {
                        demo_sensor_exit_island(this);
                        demo_reval_instance_config(this, instance);
                        //demo_set_client_config(this, instance);
                    } else {
                        DEMO_SENSOR_LOG(HIGH, this, "invalid instance! please power off");
                        sns_time timeout = sns_convert_ns_to_ticks(1000000000ULL);//1 second
                        demo_sensor_start_power_rail_timer(this, timeout, DEMO_POWER_RAIL_PENDING_DEINIT);
                    }
                    state->pwrail_pending_state = DEMO_POWER_RAIL_PENDING_NONE;
                    break;
                case DEMO_POWER_RAIL_PENDING_DEINIT://for rail off
                    DEMO_SENSOR_LOG(HIGH, this, "pwrail state = %d", state->pwrail_pending_state);
                    state->pwrail_pending_state = DEMO_POWER_RAIL_PENDING_NONE;
                    demo_sensor_update_power_rail_vote(this, SNS_RAIL_OFF, NULL);
                    break;
                default:
                    DEMO_SENSOR_LOG(HIGH, this, "unknown pwrail state = %d", state->pwrail_pending_state);
                    state->pwrail_pending_state = DEMO_POWER_RAIL_PENDING_NONE;
                    break;
                }
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "decode timer event failed!");
            }
        } else if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_REG_EVENT) {
            DEMO_SENSOR_LOG(HIGH, this, "timer successfully registered");
        } else if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_CONFIG) {
            DEMO_SENSOR_LOG(HIGH, this, "timer restart/recfg/stop");
        } else {
            DEMO_SENSOR_LOG(HIGH, this, "unknown msg id = %d", event->message_id);
        }
    }

    if (state->pwrail_pending_state == DEMO_POWER_RAIL_PENDING_NONE) {
        sns_sensor_util_remove_sensor_stream(this, &state->timer_stream);
        state->timer_stream = NULL;
        DEMO_SENSOR_LOG(HIGH, this, "free timer steam not need anymore");
    }

    return ret;
}

void demo_sensor_process_registry_event(sns_sensor *const this)
{
    sns_sensor_event *event = NULL;
    demo_state *state = (demo_state *)this->state->state;

    // if (!state->registry_stream) {
    if (!state->registry_stream ||
        !state->registry_stream->api->get_input_cnt(state->registry_stream)) {
        return;
    }
    DEMO_SENSOR_LOG(HIGH, this, "sensor %d, registry = %d, cnt = %d", state->sensor_type,
                    state->registry_stream ? 1 : -1,
                    state->registry_stream
                        ? state->registry_stream->api->get_input_cnt(state->registry_stream)
                        : 0);
    for (event = state->registry_stream->api->peek_input(state->registry_stream); event;
         event = state->registry_stream->api->get_next_input(state->registry_stream)) {
        DEMO_SENSOR_LOG(HIGH, this, "registry %d events to be processed", state->common->registry_req_cnt);
        demo_sensor_exit_island(this);
        demo_sensor_parse_registry(this, event);
    }

    demo_sensor_exit_island(this);
    if (!state->common->registry_req_cnt) {
        // demo_sensor_exit_island(this);
        // demo_update_registry_attributes(this);
        DEMO_SENSOR_LOG(HIGH, this, "sensor %d remove registry_stream", state->sensor_type);
        sns_sensor_util_remove_sensor_stream(this, &state->registry_stream);
    }
}

void demo_sensor_update_power_rail_vote(sns_sensor *const this, sns_power_rail_state vote,
                                        sns_time *on_timestamp)
{
    demo_state *state = (demo_state *)this->state->state;
    DEMO_SENSOR_LOG(HIGH, this, "vote %u -> %u", state->common->rail_cfg.rail_vote, vote);
    state->common->rail_cfg.rail_vote = vote;
    if (state->pwr_rail_service) {
        sns_rc ret = state->pwr_rail_service->api->sns_vote_power_rail_update(
            state->pwr_rail_service, this, &state->common->rail_cfg, on_timestamp);
        if (ret) {
            DEMO_SENSOR_LOG(HIGH, this, "ret = %d", ret);
        }
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "invalid pwr_rail_service!");
    }
}

sns_sensor *demo_find_sensor(sns_sensor *this, demo_sensor_type sensor_type)
{
    sns_sensor *lib_sensor = NULL;
    for (lib_sensor = this->cb->get_library_sensor(this, true); lib_sensor;
         lib_sensor = this->cb->get_library_sensor(this, false)) {
        demo_state *lib_state = (demo_state *)lib_sensor->state->state;
        if (lib_state->sensor_type == sensor_type) {
            DEMO_SENSOR_LOG(HIGH, this, "find target sensor %d", sensor_type);
            break;
        }
    }
    return lib_sensor;
}


static void demo_handle_flush_request(sns_sensor *this, sns_sensor_instance *instance)
{
    sns_sensor_event *event = NULL;
    demo_state *state = (demo_state *)this->state->state;
    flush_done_reason reason = FLUSH_TO_BE_DONE;
    demo_instance_state *istate = (demo_instance_state *)instance->state->state;
    DEMO_SENSOR_LOG(HIGH, this, "istate = 0x%p, handle flush req", istate);

    if (state->sensor_type & (DEMO_MOTION | DEMO_TEMP)) {
        reason = FLUSH_DONE_NOT_ACCEL_GYRO;
    }

    if (reason != FLUSH_TO_BE_DONE) {
        DEMO_SENSOR_LOG(HIGH, this, "istate = 0x%p, publish a FLUSH_EVENT", istate);
        sns_service_manager *sm = instance->cb->get_service_manager(instance);
        sns_event_service *event_svc = (sns_event_service *)sm->get_service(sm, SNS_EVENT_SERVICE);

        event = event_svc->api->alloc_event(event_svc, instance, 0);
        if (event) {
            //indicates no further events will be generated in response to a flush req.
            event->message_id = SNS_STD_MSGID_SNS_STD_FLUSH_EVENT;
            event->event_len = 0;
            event->timestamp = sns_get_system_time();//TODO
            // event->timestamp = (sns_get_system_time() & (~(sns_time)0x01F));  // TODO
            event_svc->api->publish_event(event_svc, instance, event, &state->my_suid);
        }
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "istate = 0x%p, set client config a FLUSH_REQ", istate);
        sns_request flush_req = {
            /* Flush a Sensor.
             * When a sensor receives a flush request it publishes any unpublished samples.
             * The sensor always publishes a SNS_STD_MSGID_SNS_STD_FLUSH_EVENT event to indicate completion of a flush request.
             * All Sensors handle this flush request message. See special case handling below empty Message
             * @event sns_pb_flush_event
            */
            .message_id = SNS_STD_MSGID_SNS_STD_FLUSH_REQ,
            .request_len = sizeof(state->sensor_type),
            .request = &(state->sensor_type)//set flush req sensor_type
        };

        /* this func will call instance_api->set_client_config */
        this->instance_api->set_client_config(instance, &flush_req);
    }
}


static sns_rc demo_handle_new_request(sns_sensor *const this, sns_sensor_instance *instance,
                                    struct sns_request const *exist_request,
                                    struct sns_request const *new_request)
{
    sns_rc ret = SNS_RC_SUCCESS;
    bool reval_config = false;
    demo_state *state = (demo_state *)this->state->state;
    demo_instance_state *istate = (demo_instance_state *)instance->state->state;

    if (exist_request) {
        DEMO_SENSOR_LOG(HIGH, this, "replace exist_req->msg_id = %d with new_request",
                        exist_request->message_id);
        instance->cb->remove_client_request(instance, exist_request);
    }

    /* add new request to request list */
    DEMO_SENSOR_LOG(HIGH, this, "add new_req->msg_id = %d to sensor %d, istate = 0x%p",
        new_request->message_id, state->sensor_type, istate);
    /* assign this instance to service the client request */
    instance->cb->add_client_request(instance, new_request);

    if (new_request->message_id == SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_CONFIG) {
        DEMO_SENSOR_LOG(HIGH, this, "STD_SENSOR_CONFIG");
        reval_config = true;
    } else {
        ret = SNS_RC_INVALID_VALUE;
        DEMO_SENSOR_LOG(HIGH, this, "unknown msg_id = %d", new_request->message_id);
    }

    /* parse above request */
    if (reval_config && state->common->rail_cfg.rail_vote != SNS_RAIL_OFF &&
        state->pwrail_pending_state == DEMO_POWER_RAIL_PENDING_NONE) {
        demo_sensor_exit_island(this);
        demo_reval_instance_config(this, instance);
    } else {
        DEMO_SENSOR_LOG(
            HIGH, this, "reject config cuz reval_config = %d, rail_cfg.rail_vote = %d, pwrail_pending_state = %d",
            reval_config, state->common->rail_cfg.rail_vote, state->pwrail_pending_state);
    }

    return ret;
}


sns_sensor_instance *demo_new_instance(sns_sensor *this)
{
    sns_sensor_instance *instance = NULL;
    demo_state          *state    = (demo_state *)this->state->state;
    //TODO: why master must be ACC? Can it be another for imu?
    sns_sensor *master_sensor = demo_find_sensor(this, DEMO_ACCEL);
    // sns_sensor *master_sensor = demo_find_sensor(this, DEMO_GYRO);
    // sns_sensor *master_sensor = this;
    sns_time   on_timestamp   = 0, delta = 0;
    sns_time   off2idle       = sns_convert_ns_to_ticks(100 * 1000 * 1000);  //100ms

    if (master_sensor) {
        demo_sensor_update_power_rail_vote(
            master_sensor, state->sensor_type == DEMO_GYRO ? SNS_RAIL_ON_NPM : SNS_RAIL_ON_LPM,
            &on_timestamp);//ts in ticks when the rail was turned ON -> it will updated by this API
        delta = sns_get_system_time() - on_timestamp;
        if (delta < off2idle) {
            DEMO_SENSOR_LOG(HIGH, this, "vote power rail will turn ON at %llu tick", on_timestamp);
            DEMO_SENSOR_LOG(HIGH, this, "off2idle = %llu tick, need start a timer to handle new state", off2idle);
            demo_sensor_start_power_rail_timer(this, off2idle - delta, DEMO_POWER_RAIL_PENDING_SET_CLIENT_REQ);
        } else {
            //state->pwrail_pending_state = DEMO_POWER_RAIL_PENDING_NONE;
            DEMO_SENSOR_LOG(HIGH, this, "rail already ON, on_timestamp = %llu, remove timer_stream", on_timestamp);
            sns_sensor_util_remove_sensor_stream(this, &state->timer_stream);
        }

        // must create instance from master sensor which includes shared_state
        // create instance will call sensor_instance's init, if init failed, create will fail
        instance = master_sensor->cb->create_instance(master_sensor, sizeof(demo_instance_state));
        if (instance) {
            DEMO_SENSOR_LOG(HIGH, this, "create instance ok");
        } else {
            DEMO_SENSOR_LOG(HIGH, this, "create instance failed!");
        }
    } else {
        DEMO_SENSOR_LOG(HIGH, this, "cannot find valid acc!");
    }
    return instance;
}

sns_sensor_instance *demo_set_client_request(sns_sensor *const this,
                                             struct sns_request const *exist_request,
                                             struct sns_request const *new_request, bool remove)
{
    demo_state *state = (demo_state *)this->state->state;
    sns_sensor_instance *instance = sns_sensor_util_get_shared_instance(this);

    DEMO_SENSOR_LOG(HIGH, this,
                    "sensor %d set exist_req.msg_id = %d, new_req.msg_id = %d, rm = %d",
                    //"demo_set_client_request:sensor %d set exist_req.msg_id = %d, new_req.msg_id = %d, rm = %d",
                    state->sensor_type, !exist_request ? -1 : exist_request->message_id,
                    !new_request ? -1 : new_request->message_id, remove);
    /* true == remove
        we need to reval instance config again because framework will send a new request,
        and we need decode it to apply a new config or not.
        For example, 200Hz acc cfg, and later 400Hz acc cfg
        Of couse we need apply 400Hz cfg to sensor chip, and framework will resample it to 200Hz client
        If 400Hz removed but 200Hz exists still, we can decoded a 200Hz cfg here, and we can apply 200Hz to sensor chip
    */
    if (remove) {
        if (instance && exist_request) {
            DEMO_SENSOR_LOG(HIGH, this, "remove exist_request");
            instance->cb->remove_client_request(instance, exist_request);
            demo_sensor_exit_island(this);
            demo_reval_instance_config(this, instance);
            if (exist_request->message_id == SNS_PHYSICAL_SENSOR_TEST_MSGID_SNS_PHYSICAL_SENSOR_TEST_CONFIG) {
                DEMO_SENSOR_LOG(HIGH, this, "self-test, pls update fac bias");
            }
        }
    } else {
    /* false == remove
        1. If new request then:
            a. Power ON rails.
            b. Power ON COM port - Instance must handle COM port power.
            c. Create new instance.
            d. Re-evaluate existing requests and choose appropriate instance config.
            e. set_client_config for this instance.
            f. Add new_request to list of requests handled by the Instance.
            g. Power OFF COM port if not needed- Instance must handle COM port power.
            h. Return the Instance.
        2. If there is an Instance already present:
            a. Add new_request to list of requests handled by the Instance.
            b. Remove exist_request from list of requests handled by the Instance.
            c. Re-evaluate existing requests and choose appropriate Instance config.
            d. set_client_config for the Instance if not the same as current config.
            e. publish the updated config.
            f. Return the Instance.
        3.  If "flush" request:
            a. Perform flush on the instance.
            b. Return NULL.
    */
        if (new_request) {
            // create new instance
            // first request cannot be a Flush request or Calibration reset request
            if (!instance && new_request->message_id != SNS_STD_MSGID_SNS_STD_FLUSH_REQ) {// need create a new instance
                instance = demo_new_instance(this);
            }
            // now/already instance present
            if (instance) {
                demo_instance_state *istate = (demo_instance_state *)instance->state->state;
                if (new_request->message_id == SNS_STD_MSGID_SNS_STD_FLUSH_REQ) {//most frequent request
                    /*
                    * Special case handling for flush request
                    * 1. No prior instance has been created, and a flush request is received, i.e.
                    *    flush request is sent before an enable req unexpected sequence, driver can
                    *    ignore the flush request, and set_client_request returns NULL
                    * 2. When enable request is sent and while that is still ongoing (phy cfg event
                    *    not published yet) and a flush request is received, expected sequence below:
                    *    a) sensor driver gets sns_std_sensor_config
                    *    b) sensor driver gets sns_std_flush_req
                    *    c) sensor driver sends sns_std_sensor_physical_config_event
                    *    d) sensor driver sends sns_std_flush_event
                    * 3. When a previous flush request is still ongoing and another flush request comes
                    *    a) sensor driver receives sns_std_flush_req
                    *    b) sensor driver receives another sns_std_flush_req while first flush request is being handled,
                    *       but no flush event is sent yet
                    *    c) sensor driver can ignore the 2nd flush request
                    *    d) sensor driver finishes processing the first flush request and sends sns_std_flush_event
                    * #1 and # 3 applies to algorithm as well.
                    */
                    DEMO_SENSOR_LOG(HIGH, this, "FLUSH_REQ, istate = 0x%p", istate);
                    if (!exist_request) {
                        DEMO_SENSOR_LOG(ERROR, this, "orphan flush req!");
                        instance = NULL;
                    } else {
                        demo_handle_flush_request(this, instance);
                    }
                    //sns_sensor_util_send_flush_event(&state->my_suid, instance);
                } else if (new_request->message_id == SNS_CAL_MSGID_SNS_CAL_RESET) {//least frequent request
                    DEMO_SENSOR_LOG(HIGH, this, "CAL_RESET, istate = 0x%p", istate);
                } else {
                    DEMO_SENSOR_LOG(HIGH, this, "msg_id = %d, istate = 0x%p", new_request->message_id, istate);
                    if(demo_handle_new_request(this, instance, exist_request, new_request)) {
                        if (exist_request) {
                            instance->cb->add_client_request(instance, exist_request);
                            DEMO_SENSOR_LOG(HIGH, this, "restore a exsit_req");
                        }
                        instance = NULL;
                    }
                }
            } else {
                DEMO_SENSOR_LOG(HIGH, this, "invalid instance!");
            }
        }
    }

    // if (!instance && !instance->cb->get_client_request(instance, &state->my_suid, true)) {
    //     DEMO_SENSOR_LOG(HIGH, this, "remove instance");
    //     this->cb->remove_instance(instance);
    // }

    return instance;
}


sns_rc demo_sensor_notify_event(sns_sensor *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;  // please always success unless fatal error, then SEE will destroy
                                  // this sensor
    //sns_sensor_event *event = NULL;
    demo_state *state = (demo_state *)this->state->state;

    DEMO_SENSOR_LOG(HIGH, this, "state = 0x%p, sensor %d", state, state->sensor_type);

    /* process suid event, and send registry req if needed */
    demo_sensor_process_suid_event(this);

    /* process registry event */
    demo_sensor_process_registry_event(this);

    /* process timer event */
    ret = demo_sensor_process_timer_event(this);

    /* start a timer for power rail and detect hw if needed */
    if (state->registry_ss_cfg_received &&
    // if (state->common->registry_md_cfg_received &&
    // if (state->registry_ss_cfg_received && state->common->registry_md_cfg_received &&
        state->common->registry_pf_cfg_received && state->common->registry_placement_received &&
        !state->hw_is_present && state->pwrail_pending_state == DEMO_POWER_RAIL_PENDING_NONE) {
        DEMO_SENSOR_LOG(HIGH, this, "sensor %d try to detect hw", state->sensor_type);
        demo_sensor_exit_island(this);
        // ret = demo_start_hw_detect(this);
        demo_start_hw_detect(this);
    }
    // DEMO_SENSOR_LOG(HIGH, this, "sensor %d, ss_cfg = %d, md_cfg = %d, pf_cfg = %d, plment = %d",
                    // state->sensor_type, state->registry_ss_cfg_received,
                    // state->common->registry_md_cfg_received,
                    // state->common->registry_pf_cfg_received,
                    // state->common->registry_placement_received);

    return ret;
}

sns_sensor_api demo_acc_sensor_api = {
    .struct_len         = sizeof(sns_sensor_api),
    .init               = demo_acc_init,
    .deinit             = demo_acc_deinit,
    .get_sensor_uid     = demo_get_sensor_uid,
    .set_client_request = demo_set_client_request,
    .notify_event       = demo_sensor_notify_event,
};

sns_sensor_api demo_gyro_sensor_api = {
    .struct_len         = sizeof(sns_sensor_api),
    .init               = demo_gyro_init,
    .deinit             = demo_gyro_deinit,
    .get_sensor_uid     = demo_get_sensor_uid,
    .set_client_request = demo_set_client_request,
    .notify_event       = demo_sensor_notify_event,
};

sns_sensor_api demo_md_sensor_api = {
    .struct_len         = sizeof(sns_sensor_api),
    .init               = demo_md_init,
    .deinit             = demo_md_deinit,
    .get_sensor_uid     = demo_get_sensor_uid,
    .set_client_request = demo_set_client_request,
    .notify_event       = demo_sensor_notify_event,
};

sns_sensor_api demo_temp_sensor_api = {
    .struct_len         = sizeof(sns_sensor_api),
    .init               = demo_temp_init,
    .deinit             = demo_temp_deinit,
    .get_sensor_uid     = demo_get_sensor_uid,
    .set_client_request = demo_set_client_request,
    .notify_event       = demo_sensor_notify_event,
};
