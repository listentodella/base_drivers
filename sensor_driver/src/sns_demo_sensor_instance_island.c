#include "sns_demo_sensor.h"
#include "sns_demo_sensor_instance.h"

void demo_instance_exit_island(sns_sensor_instance *this)
{
    sns_service_manager *sm = this->cb->get_service_manager(this);
    sns_island_service *island_svc = (sns_island_service *)sm->get_service(sm, SNS_ISLAND_SERVICE);
    island_svc->api->sensor_instance_island_exit(island_svc, this);
    DEMO_INSTANCE_LOG(HIGH, this, "demo_instance_exit_island");
}

#define IS_IRQ_LEVEL_TRIG(x) (((x) == SNS_INTERRUPT_TRIGGER_TYPE_HIGH) || \
                                      ((x) == SNS_INTERRUPT_TRIGGER_TYPE_LOW) ? true : false)
void demo_clear_irq(sns_sensor_instance *const this)
{
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // it seems we cannot access this var because of island, otherwise we need use exit_island
    /* demo_state *state = (demo_state *)istate->state;
    DEMO_INSTANCE_LOG(HIGH, this, "istate = 0x%p, state = 0x%p, trigger type = %d", istate, state,
                      state->common->irq_info.irq_config.interrupt_trigger_type); */
    //TODO:if DAE enabled, optimize it
    //only LEVEL_TRRIGERED need this msg
    if (!IS_IRQ_LEVEL_TRIG(istate->irq_info.irq_config.interrupt_trigger_type) ||
    // if (!IS_IRQ_LEVEL_TRIG(state->common->irq_info.irq_config.interrupt_trigger_type) ||
        !istate->irq_stream) {
        // DEMO_INSTANCE_LOG(HIGH, this, "tigger type = %d, irq_stream = %d",
                        //   istate->irq_info.irq_config.interrupt_trigger_type,
                        //   istate->irq_stream ? 1 : -1);
        return;
    }
    sns_request irq_msg = {
        .message_id = SNS_INTERRUPT_MSGID_SNS_INTERRUPT_IS_CLEARED,
        .request = NULL
    };

    istate->irq_stream->api->send_request(istate->irq_stream, &irq_msg);
}

static sns_rc demo_instance_process_ascp_event(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    UNUSED_VAR(this);
    // demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // DEMO_INSTANCE_LOG(HIGH, this, "0x%p", istate);
    return ret;
}

uint16_t demo_handle_irq(sns_sensor_instance *const this)
{
    uint16_t event_num = 0;
    uint32_t cnt = 0;
    pb_istream_t stream;
    sns_sensor_event *event = NULL;
    uint8_t buffer[8] = {0x00};
    sns_interrupt_event irq_event = sns_interrupt_event_init_zero;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;

    // if (!istate->irq_stream || !istate->irq_stream->api->get_input_cnt(istate->irq_stream)) {
    if (!istate->irq_stream || !(cnt = istate->irq_stream->api->get_input_cnt(istate->irq_stream))) {
        return event_num;
    }

    for (event = istate->irq_stream->api->peek_input(istate->irq_stream); event;
         event = istate->irq_stream->api->get_next_input(istate->irq_stream)) {
        DEMO_INSTANCE_LOG(HIGH, this, "event->msg_id = %d", event->message_id);
        if (event->message_id == SNS_INTERRUPT_MSGID_SNS_INTERRUPT_REG_EVENT) {
            DEMO_INSTANCE_LOG(HIGH, this, "register interrupt, please config sensor, 0x%p", istate);
            buffer[0] = 0x05;
            buffer[1] = 0x05;
            demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_INT_IO_CTRL_ADDR, buffer, 2, NULL, true);
            // buffer[0] = 0x00;
            // buffer[1] = 0x04;
            buffer[0] = 0x00;
            buffer[1] = 0x50;//wtm, full
            demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_INT_MAP2_FEAT_ADDR, buffer, 2, NULL, true);
            // buffer[0] = 0x01;
            // buffer[1] = 0x00;
            // demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_INT_LATCH_ADDR, buffer, 2, NULL, true);
        } else if (event->message_id == SNS_INTERRUPT_MSGID_SNS_INTERRUPT_EVENT) {
            DEMO_INSTANCE_LOG(HIGH, this, "interrupt triggered!");
            stream = pb_istream_from_buffer((pb_byte_t *)event->event, event->event_len);
            event_num++;
        } else {
            DEMO_INSTANCE_LOG(HIGH, this, "unknown msg id = %d", event->message_id);
        }
    }

    if (event_num) {// this is an irq triggered event
        if (pb_decode(&stream, sns_interrupt_event_fields, &irq_event)) {
            // sns_time timestamp = irq_event.timestamp;
            demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_INT_STATUS_1_ADDR, buffer, 4, NULL);
            if (buffer[0] & 0x02) {
                DEMO_INSTANCE_LOG(HIGH, this, "int_map1 motion detected! irq timestamp = %u", irq_event.timestamp);
            }
            if (buffer[1] & 0x20) {
                demo_handle_acc_drdy_irq_event(this, irq_event);
                DEMO_INSTANCE_LOG(HIGH, this, "int_map1 acc_drdy! irq timestamp = %u", irq_event.timestamp);
            }
            if (buffer[1] & 0x10) {
                demo_handle_gyr_drdy_irq_event(this, irq_event);
                DEMO_INSTANCE_LOG(HIGH, this, "int_map1 gyr_drdy! irq timestamp = %u", irq_event.timestamp);
            }
            if (buffer[1] & 0x40) {
                demo_handle_wtm_irq_event(this, irq_event);
                DEMO_INSTANCE_LOG(HIGH, this, "int_map1 fifo wtm! irq timestamp = %u", irq_event.timestamp);
            }
            if (buffer[1] & 0x80) {
                demo_handle_acc_drdy_irq_event(this, irq_event);
                DEMO_INSTANCE_LOG(HIGH, this, "int_map1 fifo full! irq timestamp = %u", irq_event.timestamp);
            }
        }
    }

    DEMO_INSTANCE_LOG(HIGH, this, "end, evt_num = %d", event_num);
    return event_num;
}

static sns_rc demo_instance_process_irq_event(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    // demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // DEMO_INSTANCE_LOG(HIGH, this, "istate = 0x%p", istate);
    // uint16_t event_cnt = demo_handle_irq(this, istate->irq_stream);
    uint16_t event_cnt = demo_handle_irq(this);
    if (event_cnt) {
        // demo_clear_irq(this, istate->irq_stream);
        demo_clear_irq(this);
    }

    return ret;
}

static sns_rc demo_instance_process_timer_event(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    sns_sensor_event *event = NULL;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // DEMO_INSTANCE_LOG(HIGH, this, "demo_instance_process_timer_event");

    for (demo_sensor_idx i = ACC; i < NUMS; i++) {
        if (!istate->ss_cfgs[i].timer_stream ||
            !istate->ss_cfgs[i].timer_stream->api->get_input_cnt(istate->ss_cfgs[i].timer_stream)) {
            continue;
        }
        for (event = istate->ss_cfgs[i].timer_stream->api->peek_input(istate->ss_cfgs[i].timer_stream); event;
             event = istate->ss_cfgs[i].timer_stream->api->get_next_input(istate->ss_cfgs[i].timer_stream)) {
            sns_timer_sensor_event timer_event;
            pb_istream_t stream =
                pb_istream_from_buffer((pb_byte_t *)event->event, event->event_len);
            if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_EVENT) {
                if (pb_decode(&stream, sns_timer_sensor_event_fields, &timer_event)) {
                    if (istate->ss_cfgs[i].timer_is_active) {
                        demo_handle_polling_timer_event(this, i, &timer_event);
                    }
                }
            } else if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_REG_EVENT) {
                DEMO_INSTANCE_LOG(HIGH, this, "timer registered successfully");
            } else if (event->message_id == SNS_TIMER_MSGID_SNS_TIMER_SENSOR_CONFIG) {
                DEMO_INSTANCE_LOG(HIGH, this, "timer config");
            } else {
                DEMO_INSTANCE_LOG(HIGH, this, "unknown event msg_id = %d", event->message_id);
            }
        }
    }

    return ret;
}

static sns_rc demo_instance_notify_event(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;

    // DEMO_INSTANCE_LOG(HIGH, this, "start");
    istate->scp_service->api->sns_scp_update_bus_power(istate->com_port.port_handle, true);

    //demo_process_dae_event();
    demo_instance_process_ascp_event(this);
    demo_instance_process_irq_event(this);
    demo_instance_process_timer_event(this);

    istate->scp_service->api->sns_scp_update_bus_power(istate->com_port.port_handle, false);
    // DEMO_INSTANCE_LOG(HIGH, this, "end");

    return ret;
}


sns_rc demo_instance_set_client_config(sns_sensor_instance *const this, sns_request const *client_request)
{
    sns_rc ret = SNS_RC_SUCCESS;
    demo_sensor_type flush_sensor = DEMO_UNKNOWN;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    DEMO_INSTANCE_LOG(HIGH, this, "start");
	if (!client_request) {
        DEMO_INSTANCE_LOG(HIGH, this, "invalid client request!");
        return SNS_RC_FAILED;
    }

    istate->scp_service->api->sns_scp_update_bus_power(istate->com_port.port_handle, true);
    switch (client_request->message_id) {
    case SNS_CAL_MSGID_SNS_CAL_RESET:
        DEMO_INSTANCE_LOG(HIGH, this, "msg id = %d -> CAL_RESET req", client_request->message_id);
        break;
    case SNS_STD_MSGID_SNS_STD_FLUSH_REQ:
        flush_sensor = *(demo_sensor_type *)client_request->request;
        DEMO_INSTANCE_LOG(HIGH, this, "msg id = %d -> FLUSH req for sensor = %d", client_request->message_id, flush_sensor);
        //TODO: many cases for flush
        if (istate->flushing_sensors) {//for case:already flushing
            istate->flushing_sensors |= flush_sensor;//append this sensor to be flushed
        } else {//for case:
            istate->flushing_sensors |= flush_sensor;//append this sensor to be flushed
            demo_send_flush_done(this, flush_sensor, FLUSH_DONE_NOT_FIFO);
            istate->flushing_sensors = 0;//TODO: at here clean?
        }
        break;
    case SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_CONFIG:
        DEMO_INSTANCE_LOG(HIGH, this, "msg id = %d -> SENSOR_CONFIG req", client_request->message_id);
        /*
         * 1. Extract sample, report rates from client_request.
         * 2. Configure sensor HW.
         * 3. sendRequest() for Timer to start/stop in case of polling using timer_stream.
         * 4. sendRequest() for Intrerupt register/de-register in case of DRI using irq_stream.
         * 5. Save the current config information like type, sample_rate, report_rate, etc.
         */
        demo_req_payload *payload = client_request->request;
        istate->sensor_enabled |= payload->sensor_type;
        // demo_hal_register_interrupt(this, payload->sensor_type);
        demo_hal_register_interrupt(this, payload->sensor_idx);
        // just test, send physical sensor config event, then instance->notify_event will be called
        demo_instance_exit_island(this);
        demo_send_config_event(this);
        // demo_dump_status(this);
        demo_instance_exit_island(this);
        demo_config_hw(this, client_request);
        break;
    case SNS_PHYSICAL_SENSOR_TEST_MSGID_SNS_PHYSICAL_SENSOR_TEST_CONFIG:
        DEMO_INSTANCE_LOG(HIGH, this, "msg id = %d -> SENSOR_TEST req", client_request->message_id);
        break;
    default:
        DEMO_INSTANCE_LOG(HIGH, this, "unknown msg id = %d!", client_request->message_id);
        break;
    }

    istate->scp_service->api->sns_scp_update_bus_power(istate->com_port.port_handle, false);
    DEMO_INSTANCE_LOG(HIGH, this, "end");
    return ret;
}


sns_sensor_instance_api demo_sensor_instance_api = {
    .struct_len        = sizeof(sns_sensor_instance_api),
    .init              = demo_instance_init,
    .deinit            = demo_instance_deinit,
    .set_client_config = demo_instance_set_client_config,
    .notify_event      = demo_instance_notify_event,
};

