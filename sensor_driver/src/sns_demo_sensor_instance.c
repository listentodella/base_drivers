#include "sns_demo_hal.h"
#include "sns_demo_sensor.h"
#include "sns_demo_sensor_instance.h"

void demo_start_sensor_polling_timer(sns_sensor_instance *this, demo_sensor_idx sensor_idx)
{
    uint8_t buffer[64] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    sns_timer_sensor_config req_payload = sns_timer_sensor_config_init_default;
    sns_request timer_req = {
        .message_id = SNS_TIMER_MSGID_SNS_TIMER_SENSOR_CONFIG,
        .request    = buffer
    };
    if (!istate->ss_cfgs[sensor_idx].timer_stream) {
        sns_service_manager *sm = this->cb->get_service_manager(this);
        sns_stream_service *stream_svc = (sns_stream_service *)sm->get_service(sm, SNS_STREAM_SERVICE);
        stream_svc->api->create_sensor_instance_stream(stream_svc, this, istate->timer_suid, &istate->ss_cfgs[sensor_idx].timer_stream);
    }

    req_payload.is_periodic = true;
    req_payload.start_time = sns_get_system_time();
    req_payload.timeout_period = istate->ss_cfgs[sensor_idx].sample_intvl;
    timer_req.request_len = pb_encode_request(buffer, sizeof(buffer), &req_payload, sns_timer_sensor_config_fields, NULL);
    if (timer_req.request_len > 0) {
        DEMO_INSTANCE_LOG(HIGH, this, "timer enable");
        istate->ss_cfgs[sensor_idx].timer_stream->api->send_request(istate->ss_cfgs[sensor_idx].timer_stream, &timer_req);
        istate->ss_cfgs[sensor_idx].timer_is_active = true;
    } else {
            DEMO_INSTANCE_LOG(HIGH, this, "invalid request len");
    }
}


void demo_set_sensor_polling_config(sns_sensor_instance *const this, demo_sensor_idx sensor_idx)
{
    demo_instance_state *istate = (demo_instance_state *)this->state->state;

    if (istate->ss_cfgs[sensor_idx].sample_intvl_req > 0) {
        if (istate->ss_cfgs[sensor_idx].sample_intvl != istate->ss_cfgs[sensor_idx].sample_intvl_req) {
            istate->ss_cfgs[sensor_idx].sample_intvl = istate->ss_cfgs[sensor_idx].sample_intvl_req;
            DEMO_INSTANCE_LOG(HIGH, this, "istate = %p, sensor %d start polling timer", istate, 1 << sensor_idx);
            demo_start_sensor_polling_timer(this, sensor_idx);
        } else {
            DEMO_INSTANCE_LOG(HIGH, this, "istate = %p, sensor %d ignore duplicated timer config", istate, 1 << sensor_idx);
        }
    } else if (istate->ss_cfgs[sensor_idx].timer_is_active) {
        istate->ss_cfgs[sensor_idx].sample_intvl = 0;
        istate->ss_cfgs[sensor_idx].timer_is_active = false;
        sns_sensor_util_remove_sensor_instance_stream(this, &istate->ss_cfgs[sensor_idx].timer_stream);
        DEMO_INSTANCE_LOG(HIGH, this, "istate = %p, sensor %d timer removed", istate, 1 << sensor_idx);
    } else {
        DEMO_INSTANCE_LOG(HIGH, this,
                          "istate = %p, sensor %d unknown status:sample_intvl_req = %lld, timer_is_active = %d",
                          istate, 1 << sensor_idx, istate->ss_cfgs[sensor_idx].sample_intvl_req,
                          istate->ss_cfgs[sensor_idx].timer_is_active);
    }
}


static uint8_t demo_compute_odr(float *sample_rate)
{
    uint8_t odr = 0x00;
    uint32_t rate = (uint32_t)(*sample_rate);

    if (rate > SENSOR_HZ(1600)) {
        rate = SENSOR_HZ(3200);
    } else if (rate > SENSOR_HZ(800)) {
        rate = SENSOR_HZ(1600);
    } else if (rate > SENSOR_HZ(400)) {
        rate = SENSOR_HZ(800);
    } else if (rate > SENSOR_HZ(200)) {
        rate = SENSOR_HZ(400);
    } else if (rate > SENSOR_HZ(100)) {
        rate = SENSOR_HZ(200);
    } else if (rate > SENSOR_HZ(50)) {
        rate = SENSOR_HZ(100);
    } else if (rate > SENSOR_HZ(25)) {
        rate = SENSOR_HZ(50);
    } else if (rate > SENSOR_HZ(25.0f / 2.0f)) {
        rate = SENSOR_HZ(25);
    } else if (rate > SENSOR_HZ(25.0f / 4.0f)) {
        rate = SENSOR_HZ(25.0f / 2.0f);
    } else if (rate > SENSOR_HZ(25.0f / 8.0f)) {
        rate = SENSOR_HZ(25.0f / 4.0f);
    } else if (rate > SENSOR_HZ(25.0f / 16.0f)) {
        rate = SENSOR_HZ(25.0f / 8.0f);
    } else if (rate > SENSOR_HZ(25.0f / 32.0f)) {
        rate = SENSOR_HZ(25.0f / 16.0f);
    } else {
        rate = SENSOR_HZ(25.0f / 32.0f);
    }
    *sample_rate = rate;

    switch (rate) {
        // fall through intended to get the correct register value
        case SENSOR_HZ(6400):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(3200):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(1600):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(800):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(400):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(200):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(100):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(50):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25.0f / 2.0f):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25.0f / 4.0f):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25.0f / 8.0f):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25.0f / 16.0f):
            odr++;
        //lint -fallthrough
        case SENSOR_HZ(25.0f / 32.0f):
            odr++;
        //lint -fallthrough
        default:
            return odr;
    }
}

static void demo_config_fifo(sns_sensor_instance *const this)
{
    uint8_t buffer[2] = {0};
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // demo_state *state = (demo_state *)istate->state;
    /* disable all fifo during config fifo in case of irq trigger */
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_FIFO_CONF_ADDR, buffer, ARR_SIZE(buffer), NULL, true);
    /* flush fifo */
    buffer[0] = 0x01;
    buffer[1] = 0x00;
    demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_FIFO_CTRL_ADDR, buffer, ARR_SIZE(buffer), NULL, true);

    buffer[0] = GET_LSB(istate->ss_cfgs[ACC].wtm * 3);
    buffer[1] = GET_MSB(istate->ss_cfgs[ACC].wtm * 3);
    demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_FIFO_WTM_ADDR, buffer, ARR_SIZE(buffer), NULL, true);

    /*TODO:only acc bit in fifo currently*/
    buffer[0] = 0x00;
    buffer[1] = 0x00;
    if (istate->sensor_enabled & DEMO_ACCEL) {
        istate->fifo_cfg.fifo_enabled |= DEMO_ACCEL;
        buffer[1] |= 0x02;
    }

    if (istate->sensor_enabled & DEMO_GYRO) {
        istate->fifo_cfg.fifo_enabled |= DEMO_GYRO;
        buffer[1] |= 0x04;
    }
    demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_FIFO_CONF_ADDR, buffer, ARR_SIZE(buffer), NULL, true);
}


//TODO: use payload or use sensor_cfg(acc/gyr_cfg)?
void demo_config_hw(sns_sensor_instance *const this, sns_request const *client_request)
{
    uint8_t odr = 0;
    demo_req_payload *payload = client_request->request;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    demo_state *state = (demo_state *)istate->state;
    // DEMO_INSTANCE_LOG(HIGH, this, "istate = %p", istate);
    DEMO_INSTANCE_LOG(HIGH, this,
                      "payload->sensor_type = %d, sample_rate = %d, report_rate = %d",
                      payload->sensor_type, payload->sample_rate, payload->report_rate);

    if (istate->sensor_enabled & DEMO_ACCEL) {
        uint8_t acc_conf[2] = {0};
        demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_ACC_CONF_ADDR, acc_conf, ARR_SIZE(acc_conf), NULL);
        // demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_ACC_CONF_ADDR, acc_conf, sizeof(acc_conf), NULL);
        // demo_com_read_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_ACC_CONF_ADDR, acc_conf, sizeof(acc_conf), NULL);
        DEMO_INSTANCE_LOG(HIGH, this,
                          "request sample rate = %d[%d, %d], get acc_conf[0x%02x, 0x%02x]",
                          (uint32_t)istate->ss_cfgs[ACC].sample_rate, state->common->pf_cfg.min_odr,
                          state->common->pf_cfg.max_odr, acc_conf[0], acc_conf[1]);
        if (payload->sample_rate < state->common->pf_cfg.min_odr) {
            payload->sample_rate = state->common->pf_cfg.min_odr;
        }
        if (istate->ss_cfgs[ACC].sample_rate > 0) {
            istate->ss_cfgs[ACC].sample_rate = SNS_MAX(istate->ss_cfgs[ACC].sample_rate, state->common->pf_cfg.min_odr);
            istate->ss_cfgs[ACC].sample_rate = SNS_MIN(istate->ss_cfgs[ACC].sample_rate, state->common->pf_cfg.max_odr);
        } else {
            istate->ss_cfgs[ACC].sample_rate = 0;
            DEMO_INSTANCE_LOG(HIGH, this, "invalid acc sample rate!");
        }
        odr = demo_compute_odr(&istate->ss_cfgs[ACC].sample_rate);
        // odr = demo_compute_odr(payload->sample_rate);
        //wtm = demo_compute_watermark(payload->sample_rate);
        istate->ss_cfgs[ACC].wtm = istate->ss_cfgs[ACC].sample_rate / istate->ss_cfgs[ACC].report_rate;

        acc_conf[0] &= ~DEMO_ACC_CONF_ODR_MASK;
        acc_conf[0] |= odr;
        if (istate->ss_cfgs[ACC].sample_rate == 0) {
        // if (payload->sample_rate == 0) {
            acc_conf[1] = 0x00;//power off
        } else if (istate->ss_cfgs[ACC].sample_rate < 15) {
        // } else if (payload->sample_rate < 15) {
            acc_conf[1] = 0x30;//low power mode
        } else if (istate->ss_cfgs[ACC].sample_rate >= 400) {
            acc_conf[1] = 0x70;//high performance mode
        } else {
            acc_conf[1] = 0x40;//normal mode
        }

        DEMO_INSTANCE_LOG(HIGH, this, "get odr = %d, wtm = %d(frames), set acc_conf[0x%02x, 0x%02x]", odr,
                          istate->ss_cfgs[ACC].wtm, acc_conf[0], acc_conf[1]);

        if (istate->ss_cfgs[ACC].ss_cfg.is_dri) {
            demo_config_fifo(this);
        }

        demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_ACC_CONF_ADDR, acc_conf, ARR_SIZE(acc_conf), NULL, true);

        if (!istate->ss_cfgs[ACC].ss_cfg.is_dri) {
            DEMO_INSTANCE_LOG(HIGH, this, "acc irq disabled, polling mode...");
            demo_set_sensor_polling_config(this, ACC);
        }
    }

    if (istate->sensor_enabled & DEMO_GYRO) {
        uint8_t gyr_conf[2] = {0};
        demo_com_read_wrapper(istate->scp_service, &istate->com_port.port_handle, DEMO_GYR_CONF_ADDR, gyr_conf, ARR_SIZE(gyr_conf), NULL);
        DEMO_INSTANCE_LOG(HIGH, this, "request sample rate = %d[%d, %d], get gyr_conf[0x%02x, 0x%02x]",
                          (uint32_t)istate->ss_cfgs[GYR].sample_rate, state->common->pf_cfg.min_odr, state->common->pf_cfg.max_odr, gyr_conf[0], gyr_conf[1]);
        if (payload->sample_rate < state->common->pf_cfg.min_odr) {
            payload->sample_rate = state->common->pf_cfg.min_odr;
        }
        if (istate->ss_cfgs[GYR].sample_rate > 0) {
            istate->ss_cfgs[GYR].sample_rate = SNS_MAX(istate->ss_cfgs[GYR].sample_rate, state->common->pf_cfg.min_odr);
            istate->ss_cfgs[GYR].sample_rate = SNS_MIN(istate->ss_cfgs[GYR].sample_rate, state->common->pf_cfg.max_odr);
        } else {
            istate->ss_cfgs[GYR].sample_rate = 0;
            DEMO_INSTANCE_LOG(HIGH, this, "invalid gyro sample rate!");
        }
        odr = demo_compute_odr(&istate->ss_cfgs[GYR].sample_rate);
        // odr = demo_compute_odr(payload->sample_rate);
        //wtm = demo_compute_watermark(payload->sample_rate);
        istate->ss_cfgs[GYR].wtm = istate->ss_cfgs[GYR].sample_rate / istate->ss_cfgs[GYR].report_rate;

        gyr_conf[0] &= ~DEMO_ACC_CONF_ODR_MASK;
        gyr_conf[0] |= odr;
        if (istate->ss_cfgs[GYR].sample_rate == 0) {
        // if (payload->sample_rate == 0) {
            gyr_conf[1] = 0x00;//power off
        } else if (istate->ss_cfgs[GYR].sample_rate < 15) {
        // } else if (payload->sample_rate < 15) {
            gyr_conf[1] = 0x30;//low power mode
        } else if (istate->ss_cfgs[GYR].sample_rate >= 400) {
        // } else if (payload->sample_rate >= 400) {
            gyr_conf[1] = 0x70;//high performance mode
        } else {
            gyr_conf[1] = 0x40;//normal mode
        }

        DEMO_INSTANCE_LOG(HIGH, this, "get odr = %d, wtm = %d(frames), set gyr_conf[0x%02x, 0x%02x]", odr,
                          istate->ss_cfgs[GYR].wtm, gyr_conf[0], gyr_conf[1]);

        if (istate->ss_cfgs[GYR].ss_cfg.is_dri) {
            demo_config_fifo(this);
        }

        demo_com_write_wrapper(istate->scp_service, istate->com_port.port_handle, DEMO_GYR_CONF_ADDR, gyr_conf, ARR_SIZE(gyr_conf), NULL, true);

        if (!istate->ss_cfgs[GYR].ss_cfg.is_dri) {
            DEMO_INSTANCE_LOG(HIGH, this, "gyro irq disabled, polling mode...");
            demo_set_sensor_polling_config(this, GYR);
        }
    }

    if (istate->sensor_enabled & DEMO_TEMP) {
        if (payload->sample_rate < state->common->pf_cfg.min_odr) {
            payload->sample_rate = state->common->pf_cfg.min_odr;
        }
        if (istate->ss_cfgs[TEMP].sample_rate > 0) {
            istate->ss_cfgs[TEMP].sample_rate = SNS_MAX(istate->ss_cfgs[TEMP].sample_rate, state->common->pf_cfg.min_odr);
            istate->ss_cfgs[TEMP].sample_rate = SNS_MIN(istate->ss_cfgs[TEMP].sample_rate, state->common->pf_cfg.max_odr);
        } else {
            istate->ss_cfgs[TEMP].sample_rate = 0;
            DEMO_INSTANCE_LOG(HIGH, this, "invalid temp sample rate!");
        }
        odr = demo_compute_odr(&istate->ss_cfgs[TEMP].sample_rate);

        if (!istate->ss_cfgs[TEMP].ss_cfg.is_dri) {
            DEMO_INSTANCE_LOG(HIGH, this, "temp irq disabled, polling mode...");
            demo_set_sensor_polling_config(this, TEMP);
        }
    }
}

// send physical sensor config event, then instance->notify_event will be called
void demo_send_config_event(sns_sensor_instance *const this)
{
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    // demo_state *state = (demo_state *)istate->state;
    sns_std_sensor_physical_config_event phy_sensor_config = sns_std_sensor_physical_config_event_init_default;

    char op_mode[] = "NORMAL";
    pb_buffer_arg op_mode_args = {
        .buf = op_mode,
        .buf_len = sizeof(op_mode)
    };

    phy_sensor_config.has_sample_rate = true;
    phy_sensor_config.has_water_mark  = false;
    // Current hardware water mark setting. 1 if FIFO not in use.
    phy_sensor_config.water_mark                  = 1;
    phy_sensor_config.operation_mode.funcs.encode = pb_encode_string_cb;
    phy_sensor_config.operation_mode.arg          = &op_mode_args;
    phy_sensor_config.has_active_current          = true;
    phy_sensor_config.has_resolution              = true;
    phy_sensor_config.range_count                 = 2;
    phy_sensor_config.has_stream_is_synchronous   = true;
    phy_sensor_config.stream_is_synchronous       = true;
    phy_sensor_config.has_dri_enabled             = true;
    phy_sensor_config.dri_enabled                 = false;
    phy_sensor_config.has_DAE_watermark           = false;
    //TODO:update config if need
    for (demo_sensor_idx i = ACC; i < NUMS; i++) {
        if (istate->sensor_enabled & (1 << i)) {
            phy_sensor_config.sample_rate = istate->ss_cfgs[i].sample_rate;
            phy_sensor_config.has_water_mark = true;
            phy_sensor_config.water_mark = istate->ss_cfgs[i].ss_cfg.is_dri?2:1;
            // phy_sensor_config.water_mark = 2;
            phy_sensor_config.has_active_current = true;
            phy_sensor_config.active_current = 180;
            phy_sensor_config.resolution = demo_ss_rsl[i][istate->ss_cfgs[i].ss_cfg.res_idx];
            phy_sensor_config.range_count = 2;
            phy_sensor_config.range[0] = demo_ss_ranges[i][istate->ss_cfgs[i].ss_cfg.res_idx].min;
            phy_sensor_config.range[1] = demo_ss_ranges[i][istate->ss_cfgs[i].ss_cfg.res_idx].max;
            phy_sensor_config.has_dri_enabled = !!(istate->ss_cfgs[i].ss_cfg.is_dri);
            phy_sensor_config.dri_enabled = !!(istate->ss_cfgs[i].ss_cfg.is_dri);
            pb_send_event(this, sns_std_sensor_physical_config_event_fields, &phy_sensor_config,
                          sns_get_system_time(),
                          SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT,
                          &istate->ss_cfgs[i].suid);
            DEMO_INSTANCE_LOG(HIGH, this, "send PCE for sensor %d:wtm = %d, sample_rate*100 = %d",
                              1 << i, phy_sensor_config.water_mark,
                              (uint32_t)phy_sensor_config.sample_rate * 100);
        }
    }
#if 0
    if (istate->sensor_enabled & DEMO_ACCEL) {
        phy_sensor_config.sample_rate        = istate->ss_cfgs[ACC].sample_rate;
        phy_sensor_config.has_water_mark     = true;
        phy_sensor_config.water_mark         = 2;
        phy_sensor_config.has_active_current = true;
        phy_sensor_config.active_current     = 180;
        phy_sensor_config.resolution = demo_accel_res[istate->ss_cfgs[ACC].ss_cfg.res_idx];
        phy_sensor_config.range_count = 2;
        phy_sensor_config.range[0] = demo_accel_ranges[istate->ss_cfgs[ACC].ss_cfg.res_idx].min;
        phy_sensor_config.range[1] = demo_accel_ranges[istate->ss_cfgs[ACC].ss_cfg.res_idx].max;
        phy_sensor_config.has_dri_enabled = !!(istate->ss_cfgs[ACC].ss_cfg.is_dri);
        phy_sensor_config.dri_enabled = !!(istate->ss_cfgs[ACC].ss_cfg.is_dri);
        pb_send_event(this, sns_std_sensor_physical_config_event_fields, &phy_sensor_config,
                      sns_get_system_time(),
                      SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT,
                      &istate->ss_cfgs[ACC].suid);
        DEMO_INSTANCE_LOG(HIGH, this, "test params for acc");
    }

    if (istate->sensor_enabled & DEMO_GYRO) {
        phy_sensor_config.sample_rate = istate->ss_cfgs[GYR].sample_rate;
        phy_sensor_config.has_water_mark = true;
        phy_sensor_config.water_mark = 2;
        phy_sensor_config.has_active_current = true;
        phy_sensor_config.active_current = 180;
        phy_sensor_config.resolution = demo_gyro_res[istate->ss_cfgs[GYR].ss_cfg.res_idx];
        phy_sensor_config.range_count = 2;
        phy_sensor_config.range[0] = demo_gyro_ranges[istate->ss_cfgs[GYR].ss_cfg.res_idx].min;
        phy_sensor_config.range[1] = demo_gyro_ranges[istate->ss_cfgs[GYR].ss_cfg.res_idx].max;
        phy_sensor_config.has_dri_enabled = !!(istate->ss_cfgs[GYR].ss_cfg.is_dri);
        phy_sensor_config.dri_enabled = !!(istate->ss_cfgs[GYR].ss_cfg.is_dri);
        pb_send_event(this, sns_std_sensor_physical_config_event_fields, &phy_sensor_config,
                      sns_get_system_time(),
                      SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT,
                      &istate->ss_cfgs[GYR].suid);
        DEMO_INSTANCE_LOG(HIGH, this, "test params for gyro");
    }

    if (istate->sensor_enabled & DEMO_TEMP) {
        phy_sensor_config.sample_rate = istate->ss_cfgs[TEMP].sample_rate;
        phy_sensor_config.has_water_mark = true;
        phy_sensor_config.water_mark = 1;
        phy_sensor_config.has_active_current = true;
        phy_sensor_config.active_current = 180;
        phy_sensor_config.resolution = demo_temp_res[istate->ss_cfgs[TEMP].ss_cfg.res_idx];
        phy_sensor_config.range_count = 2;
        phy_sensor_config.range[0] = demo_temp_ranges[istate->ss_cfgs[TEMP].ss_cfg.res_idx].min;
        phy_sensor_config.range[1] = demo_temp_ranges[istate->ss_cfgs[TEMP].ss_cfg.res_idx].max;
        phy_sensor_config.has_dri_enabled = !!(istate->ss_cfgs[TEMP].ss_cfg.is_dri);
        phy_sensor_config.dri_enabled = !!(istate->ss_cfgs[TEMP].ss_cfg.is_dri);
        pb_send_event(this, sns_std_sensor_physical_config_event_fields, &phy_sensor_config,
                      sns_get_system_time(),
                      SNS_STD_SENSOR_MSGID_SNS_STD_SENSOR_PHYSICAL_CONFIG_EVENT,
                      &istate->ss_cfgs[TEMP].suid);
        DEMO_INSTANCE_LOG(HIGH, this, "test params for temp");
    }
#endif
}


void demo_instance_clean(sns_sensor_instance *const this)
{
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    DEMO_INSTANCE_LOG(HIGH, this, "start");
    sns_sensor_util_remove_sensor_instance_stream(this, &istate->irq_stream);
    sns_sensor_util_remove_sensor_instance_stream(this, &istate->ascp_stream);
    sns_sensor_util_remove_sensor_instance_stream(this, &istate->timer_stream);

    for (demo_sensor_idx i = 0; i < NUMS; i++) {
        sns_sensor_util_remove_sensor_instance_stream(this, &istate->ss_cfgs[i].timer_stream);
    }

    if (istate->scp_service) {
        DEMO_INSTANCE_LOG(HIGH, this, "please release scp if it has been opened!");
        istate->scp_service->api->sns_scp_close(istate->com_port.port_handle);
		istate->scp_service->api->sns_scp_deregister_com_port(&istate->com_port.port_handle);
		istate->scp_service = NULL;
    }

    DEMO_INSTANCE_LOG(HIGH, this, "end");
}

sns_rc demo_instance_init(sns_sensor_instance *const this, sns_sensor_state const *sensor_state)
{
    float data[3] = {0};//imu data payload
    sns_rc ret = SNS_RC_SUCCESS;
    demo_state *state = (demo_state *)sensor_state->state;
    demo_instance_state *istate = (demo_instance_state *)this->state->state;
    sns_service_manager *sm = this->cb->get_service_manager(this);
    sns_stream_service *stream_svc = (sns_stream_service *)sm->get_service(sm, SNS_STREAM_SERVICE);

    DEMO_INSTANCE_LOG(HIGH, this, "state = 0x%p, istate = 0x%p, start", state, istate);
    sns_memzero(istate, sizeof(demo_instance_state));
    //fill istate member
    istate->state       = state;
    istate->scp_service = (sns_sync_com_port_service *)sm->get_service(sm, SNS_SYNC_COM_PORT_SERVICE);

    /* create sensor instance stream */
    sns_suid_lookup_get(&state->common->suid_lookup_data, "timer", &istate->timer_suid);
    sns_suid_lookup_get(&state->common->suid_lookup_data, "interrupt", &istate->irq_suid);
    sns_suid_lookup_get(&state->common->suid_lookup_data, "async_com_port", &istate->ascp_suid);

    for (uint8_t i = 0; i < NUMS; i++) {
        if (i == ACC || i == GYR) {
            istate->ss_cfgs[i].encoded_event_len = pb_get_encoded_size_sensor_stream_event(data, 3);
        } else {
            istate->ss_cfgs[i].encoded_event_len = pb_get_encoded_size_sensor_stream_event(&data[0], 1);
        }
        sns_memscpy(&istate->ss_cfgs[i].suid, sizeof(sns_sensor_uid), &(suids[i][state->hw_idx]), sizeof(sns_sensor_uid));
        // sns_memscpy(&istate->ss_cfgs[i].suid, sizeof(sns_sensor_uid), &(istate->state->common->states[i]->my_suid), sizeof(sns_sensor_uid));
        istate->ss_cfgs[i].ss_cfg = state->common->states[i]->ss_cfg;
        DEMO_INSTANCE_LOG(HIGH, this, "sensor %d encoded_event_len = %d", 1 << i, istate->ss_cfgs[i].encoded_event_len);
        ret = stream_svc->api->create_sensor_instance_stream(stream_svc, this, istate->timer_suid, &istate->ss_cfgs[i].timer_stream);
        if (ret) {
            DEMO_INSTANCE_LOG(HIGH, this, "sensor %d failed to create timer_stream, ret = %d!", 1 << i, ret);
        } else {
            DEMO_INSTANCE_LOG(HIGH, this, "sensor %d get timer_stream = 0x%p, timer_suid = %"PRIsuid, 1 << i,
                              istate->ss_cfgs[i].timer_stream, SNS_PRI_SUID(&istate->timer_suid));
        }
    }

    ret = stream_svc->api->create_sensor_instance_stream(stream_svc, this, istate->irq_suid, &istate->irq_stream);
    if (ret) {
        DEMO_INSTANCE_LOG(HIGH, this, "failed to create irq_stream, ret = %d!", ret);
    } else {
        DEMO_INSTANCE_LOG(HIGH, this, "get irq_stream = 0x%p, suid = %"PRIsuid,
                          istate->irq_stream, SNS_PRI_SUID(&istate->irq_suid));
    }
    ret = stream_svc->api->create_sensor_instance_stream(stream_svc, this, istate->ascp_suid, &istate->ascp_stream);
    if (ret) {
        DEMO_INSTANCE_LOG(HIGH, this, "failed to create ascp_stream, ret = %d!", ret);
    } else {
        DEMO_INSTANCE_LOG(HIGH, this, "get ascp_stream = 0x%p, suid = %"PRIsuid,
                          istate->ascp_stream, SNS_PRI_SUID(&istate->ascp_suid));
    }
    ret = stream_svc->api->create_sensor_instance_stream(stream_svc, this, istate->timer_suid, &istate->timer_stream);
    if (ret) {
        DEMO_INSTANCE_LOG(HIGH, this, "failed to create timer_stream, ret = %d!", ret);
    } else {
        DEMO_INSTANCE_LOG(HIGH, this, "get timer_stream = 0x%p, suid = %"PRIsuid,
                          state->timer_stream, SNS_PRI_SUID(&istate->timer_suid));
    }
    // stream_svc->api->create_sensor_instance_stream(stream_svc, this, other suid ...);

    /* init irq config used by intance */
    istate->irq_info = state->common->irq_info;
    istate->irq_info.irq_ready = false;
    istate->irq_info.irq_registered = false;

    /* init COM port to be used by instance */
    istate->com_port = state->common->com_port;
    istate->scp_service->api->sns_scp_register_com_port(&istate->com_port.com_config, &istate->com_port.port_handle);
    if (!istate->com_port.port_handle || !istate->ascp_stream || !istate->irq_stream) {
        demo_instance_clean(this);
        return SNS_RC_FAILED;
    }

    istate->scp_service->api->sns_scp_open(istate->com_port.port_handle);
    istate->scp_service->api->sns_scp_update_bus_power(istate->com_port.port_handle, false);
    /* config Async com port */
    {
        uint8_t pb_encode_buffer[100];
        sns_data_stream *data_stream = istate->ascp_stream;
        sns_com_port_config *com_config = &state->common->com_port.com_config;
        sns_request ascp_request = {
            .message_id = SNS_ASYNC_COM_PORT_MSGID_SNS_ASYNC_COM_PORT_CONFIG,
            .request = &pb_encode_buffer
        };
        istate->ascp_config.bus_type = (com_config->bus_type == SNS_BUS_I2C)
                                           ? SNS_ASYNC_COM_PORT_BUS_TYPE_I2C
                                           : SNS_ASYNC_COM_PORT_BUS_TYPE_SPI;
        istate->ascp_config.slave_control     = com_config->slave_control;
        istate->ascp_config.reg_addr_type     = SNS_ASYNC_COM_PORT_REG_ADDR_TYPE_8_BIT;
        istate->ascp_config.min_bus_speed_kHz = com_config->min_bus_speed_KHz;
        istate->ascp_config.max_bus_speed_kHz = com_config->max_bus_speed_KHz;
        istate->ascp_config.bus_instance      = com_config->bus_instance;

        ascp_request.request_len = pb_encode_request(pb_encode_buffer, sizeof(pb_encode_buffer), &istate->ascp_config, sns_async_com_port_config_fields, NULL);
        data_stream->api->send_request(data_stream, &ascp_request);
    }

    /* init chip register config */
    DEMO_INSTANCE_LOG(HIGH, this, "init chip register config");
    demo_hal_reset_device(this);

    //TODO:if DAE enabled, please optimize it
    // demo_hal_register_interrupt(this);

    DEMO_INSTANCE_LOG(HIGH, this, "end");
    //ret = SNS_RC_FAILED;//this will result in the failure of create_instance
    return ret;
}

sns_rc demo_instance_deinit(sns_sensor_instance *const this)
{
    sns_rc ret = SNS_RC_SUCCESS;
    DEMO_INSTANCE_LOG(HIGH, this, "demo_instance_deinit");
    demo_instance_clean(this);
    return ret;
}




