#include "sns_demo_hal.h"
#include "sns_demo_sensor.h"
#include "sns_demo_sensor_instance.h"

sns_rc demo_com_read_wrapper(sns_sync_com_port_service *scp_service,
                             sns_sync_com_port_handle **port_handle,
                            //  sns_sync_com_port_handle *port_handle,
                             uint32_t addr, uint8_t *buffer,
                             uint32_t size, uint32_t *xfer_size)
{
    sns_rc ret = SNS_RC_SUCCESS;
    com_port_info *com_port =
        container_of(sns_sync_com_port_handle *, port_handle, com_port_info, port_handle);

    uint8_t temp_buf[size + com_port->dummy_byte];
    sns_port_vector port_vec = {
        .reg_addr = addr,
        .buffer = temp_buf,
        .bytes = ARR_SIZE(temp_buf),
        .is_write = false
    };

    if (!port_handle) {
        return SNS_RC_INVALID_VALUE;
    }

    if (!scp_service) {
        return SNS_RC_NOT_AVAILABLE;
    }

    // ret = scp_service->api->sns_scp_register_rw(port_handle, &port_vec, 1, false, xfer_size);
    ret = scp_service->api->sns_scp_register_rw(*port_handle, &port_vec, 1, false, xfer_size);

    sns_memscpy(buffer, size * sizeof(uint8_t), (temp_buf + com_port->dummy_byte), size * sizeof(uint8_t));

    return ret;
}

sns_rc demo_com_write_wrapper(sns_sync_com_port_service *scp_service,
                             sns_sync_com_port_handle *port_handle,
                             uint32_t addr, uint8_t *buffer,
                             uint32_t size, uint32_t *xfer_size,
                             bool save_write_time)
{
    sns_port_vector port_vec = {
        .reg_addr = addr,
        .buffer = buffer,
        .bytes = size,
        .is_write = true
    };

    if (!port_handle) {
        return SNS_RC_INVALID_VALUE;
    }

    if (!scp_service) {
        return SNS_RC_NOT_AVAILABLE;
    }

    return scp_service->api->sns_scp_register_rw(port_handle, &port_vec, 1, save_write_time, xfer_size);
}


sns_rc demo_switch_bus(sns_bus_type bus_type, sns_sync_com_port_service *scp_service, sns_sync_com_port_handle **port_handle)
{
    uint8_t buffer[2] = {0};
    sns_rc ret = SNS_RC_SUCCESS;

    if (bus_type == SNS_BUS_SPI) {
        // DEMO_SENSOR_LOG(HIGH, this, "switch spi bus");
        ret = demo_com_read_wrapper(scp_service, port_handle, DEMO_CHIP_ID_ADDR, buffer,
                                    ARR_SIZE(buffer), NULL);
        //this is mandatory otherwise sensor cannot switch to spi
        udelay(2000);
    } else if (bus_type == SNS_BUS_I3C) {//TODO
        // DEMO_SENSOR_LOG(HIGH, this, "switch i3c bus");
    } else if (bus_type == SNS_BUS_I2C) {//TODO
        // DEMO_SENSOR_LOG(HIGH, this, "switch i2c bus");
    }
    return ret;
}


sns_rc demo_read_chip_id(sns_sync_com_port_service *scp_service,
                         sns_sync_com_port_handle **port_handle, uint8_t *buffer)
{
    sns_rc ret = SNS_RC_SUCCESS;
    ret = demo_com_read_wrapper(scp_service, port_handle, DEMO_CHIP_ID_ADDR, buffer, 2, NULL);
    return ret;
}

void demo_dump_status(sns_sensor_instance *const this)
{
    uint8_t buffer[8] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_ACC_CONF_ADDR, buffer, 2, NULL);
    DEMO_INSTANCE_LOG(HIGH, this, "read acc_conf = [0x%02x, 0x%02x]", buffer[0], buffer[1]);

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_INT_IO_CTRL_ADDR, buffer, 2, NULL);
    DEMO_INSTANCE_LOG(HIGH, this, "read int_io_ctrl = [0x%02x, 0x%02x]", buffer[0], buffer[1]);

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_INT_MAP2_FEAT_ADDR, buffer, 2, NULL);
    DEMO_INSTANCE_LOG(HIGH, this, "read int_map2 = [0x%02x, 0x%02x]", buffer[0], buffer[1]);

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_INT_LATCH_ADDR, buffer, 2, NULL);
    DEMO_INSTANCE_LOG(HIGH, this, "read latch mode = [0x%02x, 0x%02x]", buffer[0], buffer[1]);
}


sns_rc demo_hal_reset_device(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    uint8_t buffer[2] = {0xAF, 0xDE};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    if (!istate->reset_done || istate->force_reset) {
        demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_CMD_ADDR, buffer, ARR_SIZE(buffer), NULL, true);
        DEMO_INSTANCE_LOG(HIGH, this, "remember to clean related var/states!!!");
        //demo_clean_xxx
        istate->force_reset = false;
        istate->reset_done = true;
        udelay(2000);
        DEMO_INSTANCE_LOG(HIGH, this, "reset done");
    } else {
        DEMO_INSTANCE_LOG(HIGH, this, "already reset...");
    }

    return ret;
}

sns_rc demo_hal_send_cmd(sns_sensor_instance *const this, demo_cmd_t cmd)
{
    sns_rc ret = SNS_RC_SUCCESS;
    // demo_instance_state *istate = (demo_instance_state *)this->state->state;

    switch (cmd) {
    case DEMO_CMD_SOFT_RESET:
        DEMO_INSTANCE_LOG(HIGH, this, "command = %d -> soft reset", cmd);
        ret = demo_hal_reset_device(this);
        break;
    case DEMO_CMD_SELF_TEST:
        DEMO_INSTANCE_LOG(HIGH, this, "command = %d -> self test", cmd);
        break;
    default:
        DEMO_INSTANCE_LOG(HIGH, this, "unknown command = %d!", cmd);
        break;
    }

    return ret;
}

void demo_handle_polling_timer_event(sns_sensor_instance *const this, demo_sensor_idx sensor_idx,
                                     sns_timer_sensor_event *const timer_event)
{
    uint8_t addr = 0x00;
    int16_t data[3] = {0};
    float temp_val = 0.0f;
    float fac_cal_bias[3] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *state = istate->state->common->states[sensor_idx];

    sns_time ts = sns_get_system_time();
    sns_std_sensor_sample_status status = SNS_STD_SENSOR_SAMPLE_STATUS_ACCURACY_HIGH;

    switch (sensor_idx) {
    case ACC:
        addr = DEMO_ACC_X_ADDR;
        break;
    case GYR:
        addr = DEMO_GYR_X_ADDR;
        break;
    case TEMP:
        addr = DEMO_TEMP_ADDR;
        break;
    default:
        DEMO_INSTANCE_LOG(HIGH, this, "unknown sensor idx = %d!", sensor_idx);
        break;
    }

    if (sensor_idx == ACC || sensor_idx == GYR) {
        uint8_t buffer[6];
        demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, addr, buffer, ARR_SIZE(buffer), NULL);
        data[0] = (int16_t)(buffer[0] | buffer[1] << 8);
        data[1] = (int16_t)(buffer[2] | buffer[3] << 8);
        data[2] = (int16_t)(buffer[4] | buffer[5] << 8);
        DEMO_INSTANCE_LOG(HIGH, this, "sensor %d get data = [0x%04x, 0x%04x, 0x%04x], timeout = %llu", 1 << sensor_idx,
                          data[0], data[1], data[2], timer_event->timeout_time);

        vector3 data_cal = {
            .x = (data[0] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[0]) * state->scale_factor,
            .y = (data[1] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[1]) * state->scale_factor,
            .z = (data[2] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[2]) * state->scale_factor
        };

#if 0
    float f_data[3] = {data[0] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[1] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[2] * state->resolutions[state->res_idx] / state->scale_factor};

    data_cal = sns_apply_calibration_correction_3(make_vector3_from_array(f_data),
                                                  make_vector3_from_array(fac_cal_bias),
                                                  state->fac_cal_corr_mat);
#endif
    pb_send_sensor_stream_event(this, &istate->ss_cfgs[sensor_idx].suid, ts,
                                SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT, status, data_cal.data,
                                ARR_SIZE(data_cal.data), istate->ss_cfgs[sensor_idx].encoded_event_len);
    } else if (sensor_idx == TEMP) {
        uint8_t buffer[2];
        demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, addr, buffer, ARR_SIZE(buffer), NULL);
        if (buffer[0] == 0 && buffer[1] == 0x80) {
            temp_val = 25.0f;  // default value
        } else {
            data[0] = (buffer[0] | buffer[1] << 8);
            // data[0] = (int16_t) (buffer[0] | buffer[1] << 8);
            temp_val = data[0] * state->scale_factor + 23.0f;
        }
        DEMO_INSTANCE_LOG(HIGH, this, "temp * 1000 = %d[0x%02x, 0x%02x], timeout = %llu",
                          (int32_t)(temp_val * 1000), buffer[0], buffer[1],
                          timer_event->timeout_time);
        temp_val = state->fac_cal_corr_mat.e00 * (temp_val - (state->fac_cal_bias[0] / DEMO_SCALE_FACTOR_DATA_TEMP));
    pb_send_sensor_stream_event(this, &istate->ss_cfgs[TEMP].suid, ts,
                                SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT, status, &temp_val,
                                1, istate->ss_cfgs[TEMP].encoded_event_len);
    }

}


void demo_handle_acc_drdy_irq_event(sns_sensor_instance *const this, sns_interrupt_event irq_event)
{
    uint8_t buffer[6] = {0};
    int16_t data[3] = {0};
    float fac_cal_bias[3] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *state = istate->state->common->states[ACC];
    sns_std_sensor_sample_status status = SNS_STD_SENSOR_SAMPLE_STATUS_ACCURACY_HIGH;

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_ACC_X_ADDR, buffer, ARR_SIZE(buffer), NULL);
    data[0] = (int16_t) (buffer[0] | buffer[1] << 8);
    data[1] = (int16_t) (buffer[2] | buffer[3] << 8);
    data[2] = (int16_t) (buffer[4] | buffer[5] << 8);
    DEMO_INSTANCE_LOG(HIGH, this, "get data = [0x%04x, 0x%04x, 0x%04x], timestamp = %d",
                    data[0], data[1], data[2], irq_event.timestamp);

    vector3 data_cal = {
        .x = (data[0] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[0]) * state->scale_factor,
        .y = (data[1] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[1]) * state->scale_factor,
        .z = (data[2] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[2]) * state->scale_factor
    };

    sns_time ts = sns_get_system_time();
    pb_send_sensor_stream_event(this, &istate->ss_cfgs[ACC].suid, ts,
                                SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT, status, data_cal.data,
                                ARR_SIZE(data_cal.data), istate->ss_cfgs[ACC].encoded_event_len);
}

void demo_handle_gyr_drdy_irq_event(sns_sensor_instance *const this, sns_interrupt_event irq_event)
{
    uint8_t buffer[6] = {0};
    int16_t data[3] = {0};
    float fac_cal_bias[3] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *state = istate->state->common->states[GYR];
    sns_std_sensor_sample_status status = SNS_STD_SENSOR_SAMPLE_STATUS_ACCURACY_HIGH;

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_GYR_X_ADDR, buffer, ARR_SIZE(buffer), NULL);
    data[0] = (int16_t) (buffer[0] | buffer[1] << 8);
    data[1] = (int16_t) (buffer[2] | buffer[3] << 8);
    data[2] = (int16_t) (buffer[4] | buffer[5] << 8);
    DEMO_INSTANCE_LOG(HIGH, this, "get data = [0x%04x, 0x%04x, 0x%04x], timestamp = %d",
                    data[0], data[1], data[2], irq_event.timestamp);

    vector3 data_cal = {
        .x = (data[0] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[0]) * state->scale_factor,
        .y = (data[1] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[1]) * state->scale_factor,
        .z = (data[2] * state->resolutions[state->ss_cfg.res_idx] - fac_cal_bias[2]) * state->scale_factor
    };

    sns_time ts = sns_get_system_time();
    pb_send_sensor_stream_event(this, &istate->ss_cfgs[GYR].suid, ts,
                                SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT, status, data_cal.data,
                                ARR_SIZE(data_cal.data), istate->ss_cfgs[GYR].encoded_event_len);
}



void demo_handle_wtm_irq_event(sns_sensor_instance *const this, sns_interrupt_event irq_event)
{
    uint8_t buffer[2] = {0};
    int16_t data[3] = {0};
    uint16_t fifo_len = 0;
    float fac_cal_bias[3] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *acc_state = istate->state->common->states[ACC];
    demo_state *gyr_state = istate->state->common->states[GYR];
    // UNUSED_VAR(irq_event);
    sns_std_sensor_sample_status status = SNS_STD_SENSOR_SAMPLE_STATUS_ACCURACY_HIGH;

    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_FIFO_LENGTH_ADDR, buffer, 2, NULL);
    fifo_len = (buffer[0] | buffer[1] << 8) * 2;//bytes

    uint8_t buf[fifo_len];
    sns_memzero(buf, sizeof(buf));
    demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_FIFO_DATA_ADDR, buf, fifo_len, NULL);

    size_t i = 0;
    uint8_t acc_event_size = 0, gyr_event_size = 0;
    while (fifo_len > 0) {
        if (buf[i] == 0x00 && buf[i + 1] == 0x80) {
            break;
        }

        // handle acc data
        if (istate->fifo_cfg.fifo_enabled & DEMO_ACCEL) {
            // osLog(LOG_ERROR, " fifo for acc enabled\n");
            if (fifo_len >= 6) {
                // skip invalid/dummy acc data (0x7f01, 0x8000, 0x8000), which cost 3 words
                if (buf[i] == 0x01 && buf[i + 1] == 0x7f) {
                    i += 6;
                    fifo_len -= 6;
                    goto skip_acc;
                }
                data[0] = (int16_t)(buf[i + 0] | buf[i + 1] << 8);
                data[1] = (int16_t)(buf[i + 2] | buf[i + 3] << 8);
                data[2] = (int16_t)(buf[i + 4] | buf[i + 5] << 8);
                DEMO_INSTANCE_LOG(HIGH, this, "get acc data = [0x%04x, 0x%04x, 0x%04x], timestamp = %lld",
                                  data[0], data[1], data[2], irq_event.timestamp);

                vector3 data_cal = {
                    .x = (data[0] * acc_state->resolutions[acc_state->ss_cfg.res_idx] - fac_cal_bias[0]) * acc_state->scale_factor,
                    .y = (data[1] * acc_state->resolutions[acc_state->ss_cfg.res_idx] - fac_cal_bias[1]) * acc_state->scale_factor,
                    .z = (data[2] * acc_state->resolutions[acc_state->ss_cfg.res_idx] - fac_cal_bias[2]) * acc_state->scale_factor
                };
#if 0
    float f_data[3] = {data[0] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[1] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[2] * state->resolutions[state->res_idx] / state->scale_factor};

    data_cal = sns_apply_calibration_correction_3(make_vector3_from_array(f_data),
                                                  make_vector3_from_array(fac_cal_bias),
                                                  state->fac_cal_corr_mat);
#endif
                sns_time ts = sns_get_system_time();
                pb_send_sensor_stream_event(
                    this, &istate->ss_cfgs[ACC].suid, ts, SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT,
                    status, data_cal.data, ARR_SIZE(data_cal.data), istate->ss_cfgs[ACC].encoded_event_len);

                i += 6;
                fifo_len -= 6;
                acc_event_size++;
            } else {
                fifo_len = 0;  // drop this data
                // osLog(LOG_ERROR, " no enough data for acc\n");
            }

        }
skip_acc:
        if (istate->fifo_cfg.fifo_enabled & DEMO_GYRO) {
            if (fifo_len >= 6) {
                // skip invalid/dummy gyro data (0x7f02, 0x8000, 0x8000), which cost 3 words
                if (buf[i] == 0x02 && buf[i + 1] == 0x7f) {
                    i += 6;
                    fifo_len -= 6;
                    goto skip_gyr;
                }
                data[0] = (int16_t)(buf[i + 0] | buf[i + 1] << 8);
                data[1] = (int16_t)(buf[i + 2] | buf[i + 3] << 8);
                data[2] = (int16_t)(buf[i + 4] | buf[i + 5] << 8);
                DEMO_INSTANCE_LOG(HIGH, this, "get gyro data = [0x%04x, 0x%04x, 0x%04x], timestamp = %lld",
                                  data[0], data[1], data[2], irq_event.timestamp);

                vector3 data_cal = {
                    .x = (data[0] * gyr_state->resolutions[gyr_state->ss_cfg.res_idx] - fac_cal_bias[0]) * gyr_state->scale_factor,
                    .y = (data[1] * gyr_state->resolutions[gyr_state->ss_cfg.res_idx] - fac_cal_bias[1]) * gyr_state->scale_factor,
                    .z = (data[2] * gyr_state->resolutions[gyr_state->ss_cfg.res_idx] - fac_cal_bias[2]) * gyr_state->scale_factor
                };
#if 0
    float f_data[3] = {data[0] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[1] * state->resolutions[state->res_idx] / state->scale_factor,
                       data[2] * state->resolutions[state->res_idx] / state->scale_factor};

    data_cal = sns_apply_calibration_correction_3(make_vector3_from_array(f_data),
                                                  make_vector3_from_array(fac_cal_bias),
                                                  state->fac_cal_corr_mat);
#endif

                sns_time ts = sns_get_system_time();
                pb_send_sensor_stream_event(
                    this, &istate->ss_cfgs[GYR].suid, ts, SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_EVENT,
                    status, data_cal.data, ARR_SIZE(data_cal.data), istate->ss_cfgs[GYR].encoded_event_len);

                i += 6;
                fifo_len -= 6;
                gyr_event_size++;
            } else {
                fifo_len = 0;
            }
        }
skip_gyr:
        if (buf[i] == 0x00 && buf[i + 1] == 0x80) {
            break;
        }
    }
}


// void demo_hal_register_interrupt(sns_sensor_instance *this, demo_sensor_type sensor_type)
void demo_hal_register_interrupt(sns_sensor_instance *this, demo_sensor_idx sensor_idx)
{
    // demo_state *state = NULL;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *state = istate->state->common->states[sensor_idx];

    if (!state->ss_cfg.is_dri) {
        DEMO_INSTANCE_LOG(HIGH, this, "sensor %d hw_idx[%d], istate = 0x%p, run in polling mode",
                          state->sensor_type, state->hw_idx, istate);
        return;
    }

    DEMO_INSTANCE_LOG(HIGH, this, "sensor %d hw_idx[%d], istate = 0x%p, irq_stream = 0x%p, irq cfg = [%d, %d, %d, %d, %d]",
                      state->sensor_type, state->hw_idx, istate, istate->irq_stream,
                      istate->irq_info.irq_config.interrupt_num,
                      istate->irq_info.irq_config.interrupt_pull_type,
                      istate->irq_info.irq_config.is_chip_pin,
                      istate->irq_info.irq_config.interrupt_drive_strength,
                      istate->irq_info.irq_config.interrupt_trigger_type);

    if (!(istate->irq_info.irq_registered)) {
        uint8_t buffer[20];
        sns_request irq_req = {
            .message_id = SNS_INTERRUPT_MSGID_SNS_INTERRUPT_REQ,
            .request = buffer
        };

        irq_req.request_len =
            // pb_encode_request(buffer, sizeof(buffer), &state->common->irq_info.irq_config,
            pb_encode_request(buffer, sizeof(buffer), &istate->irq_info.irq_config,
                              sns_interrupt_req_fields, NULL);
        if (irq_req.request_len > 0) {
            sns_rc ret = istate->irq_stream->api->send_request(istate->irq_stream, &irq_req);
            if (ret) {
                DEMO_INSTANCE_LOG(HIGH, this, "failed to send reqeust, ret = %d", ret);
            }
            istate->irq_info.irq_registered = true;
        }
    }
}

void demo_send_flush_done(sns_sensor_instance *this, demo_sensor_type flush_sensors,
                          flush_done_reason reason)
{
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    DEMO_INSTANCE_LOG(HIGH, this, "flushing sensors = 0x%x, FLUSH_EVENT reason = %d", istate->flushing_sensors, reason);
    while (flush_sensors) {
        sns_sensor_uid const *suid = NULL;
        demo_sensor_type sensor_type = DEMO_UNKNOWN;
        if (flush_sensors & DEMO_ACCEL) {
            sensor_type = DEMO_ACCEL;
            suid = &istate->ss_cfgs[ACC].suid;
        } else {//TODO:fill other sensors
            flush_sensors = 0;
        }

        if (suid) {
            sns_service_manager *sm = this->cb->get_service_manager(this);
            sns_event_service *event_svc = (sns_event_service *)sm->get_service(sm, SNS_EVENT_SERVICE);
            sns_sensor_event *event = event_svc->api->alloc_event(event_svc, this, 0);
            event->message_id = SNS_STD_MSGID_SNS_STD_FLUSH_EVENT;
            event->event_len = 0;
            event->timestamp = sns_get_system_time();
            flush_sensors &= ~sensor_type;//clear this sensor_type
            event_svc->api->publish_event(event_svc, this, event, suid);
        }
    }
}