/**
 * @file telemetry.c
 * @brief Telemetry Sampling Orchestrator and Payload Builder Implementation
 */

#include "telemetry.h"
#include "drv_tmp117.h"
#include "drv_vbat.h"
#include "drv_lis2dw12.h"
#include "retention.h"
#include "wake_mgr.h"
#include "ble_ulp_svc.h"
#include <string.h>

static uint8_t s_telemetry_seq = 0;
static bool s_motion_occurred = false;

void telemetry_init(void)
{
    s_telemetry_seq = 0;
    s_motion_occurred = false;
}

ret_code_t telemetry_sample_and_build(uint8_t pwr_state, telemetry_frame_t * p_frame_out)
{
    if (!p_frame_out) {
        return NRF_ERROR_NULL;
    }

    memset(p_frame_out, 0, sizeof(telemetry_frame_t));

    /* 1. Header */
    p_frame_out->ver_type = (uint8_t)((PROTOCOL_VERSION << 4) | (MSG_TYPE_TELEMETRY & 0x0F));
    p_frame_out->seq = s_telemetry_seq++;
    p_frame_out->len = 15;
    p_frame_out->pwr_state = pwr_state;
    p_frame_out->wake_reason = (uint8_t)wake_mgr_get_last_reason();

    /* 2. Sample Sensors */
    int16_t temp_c_x100 = 2500;
    ret_code_t err_temp = drv_tmp117_sample_temp_c_x100(&temp_c_x100);
    p_frame_out->temp_c_x100 = temp_c_x100;

    uint16_t vbat_mv = 3000;
    ret_code_t err_vbat = drv_vbat_sample_mv(&vbat_mv);
    p_frame_out->vbat_mv = vbat_mv;

    /* 3. Evaluate Flags */
    uint8_t flags = 0;
    if (drv_vbat_get_level() != VBAT_LEVEL_OK) {
        flags |= TELEM_FLAG_LOW_BAT;
    }
    if (err_temp != NRF_SUCCESS || err_vbat != NRF_SUCCESS) {
        flags |= TELEM_FLAG_SENSOR_FAULT;
    }

    const retention_record_t * p_ret = retention_get_record();
    if (p_ret && p_ret->boot_cnt > 1) {
        flags |= TELEM_FLAG_RETENTION_REST;
    }

    if (s_motion_occurred || drv_lis2dw12_is_motion_detected()) {
        flags |= TELEM_FLAG_MOTION_SINCE_LST;
        s_motion_occurred = false; /* Clear flag after reporting */
    }
    p_frame_out->flags = flags;

    /* 4. Retained Counters */
    if (p_ret) {
        p_frame_out->motion_cnt = p_ret->motion_cnt;
        p_frame_out->sysoff_cnt = p_ret->sysoff_cnt;
        p_frame_out->uptime_s   = p_ret->uptime_s;
    }

    /* 5. Finalize with CRC-16 */
    uint8_t wire_frame[TELEMETRY_FRAME_SIZE];
    protocol_encode_telemetry(p_frame_out, wire_frame);
    p_frame_out->crc16 = (uint16_t)(wire_frame[18] | ((uint16_t)wire_frame[19] << 8));

    return NRF_SUCCESS;
}

ret_code_t telemetry_send_notification(uint16_t conn_handle)
{
    telemetry_frame_t frame;
    ret_code_t err = telemetry_sample_and_build(3 /* S_CONN_ACTIVE */, &frame);
    if (err != NRF_SUCCESS) {
        return err;
    }

    uint8_t wire_buf[TELEMETRY_FRAME_SIZE];
    protocol_encode_telemetry(&frame, wire_buf);

    return ble_ulp_svc_notify_telemetry(conn_handle, wire_buf, TELEMETRY_FRAME_SIZE);
}
