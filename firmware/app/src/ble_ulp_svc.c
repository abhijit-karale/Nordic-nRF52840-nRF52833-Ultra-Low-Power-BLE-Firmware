/**
 * @file ble_ulp_svc.c
 * @brief ULP Custom GATT Service Implementation
 */

#include "ble_ulp_svc.h"
#include "app_cfg.h"
#include "diag.h"
#include <string.h>

static uint16_t s_service_handle = 0;
static ble_gatts_char_handles_t s_telemetry_handles;
static ble_gatts_char_handles_t s_config_handles;
static ble_gatts_char_handles_t s_control_handles;
static ble_gatts_char_handles_t s_status_handles;

static uint8_t s_uuid_type = BLE_UUID_TYPE_VENDOR_BEGIN;
static bool s_telemetry_notify_enabled = false;
static ble_ulp_control_handler_t s_control_callback = NULL;

static uint8_t s_status_buffer[sizeof(status_payload_t)];

ret_code_t ble_ulp_svc_init(ble_ulp_control_handler_t control_handler)
{
    s_control_callback = control_handler;
    s_telemetry_notify_enabled = false;

    ble_uuid128_t base_uuid = { ULP_SVC_UUID_BASE };
    ret_code_t err = sd_ble_uuid_vs_add(&base_uuid, &s_uuid_type);
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) {
        return err;
    }

    ble_uuid_t service_uuid = {
        .uuid = ULP_SVC_UUID_SERVICE,
        .type = s_uuid_type
    };

    err = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &service_uuid, &s_service_handle);
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) {
        return err;
    }

    /* 1. Add Telemetry Characteristic (Notify, 20 bytes) */
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.notify = 1;

    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    char_md.p_cccd_md = &cccd_md;

    ble_uuid_t telem_uuid = {
        .uuid = ULP_SVC_UUID_TELEMETRY,
        .type = s_uuid_type
    };

    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    ble_gatts_attr_t attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &telem_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = TELEMETRY_FRAME_SIZE;
    attr_char_value.max_len  = TELEMETRY_FRAME_SIZE;

    sd_ble_gatts_characteristic_add(s_service_handle, &char_md, &attr_char_value, &s_telemetry_handles);

    /* 2. Add Config Characteristic (Read/Write, 15 bytes) */
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read  = 1;
    char_md.char_props.write = 1;

    ble_uuid_t config_uuid = {
        .uuid = ULP_SVC_UUID_CONFIG,
        .type = s_uuid_type
    };

    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &config_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = CONFIG_FRAME_SIZE;
    attr_char_value.max_len  = CONFIG_FRAME_SIZE;

    sd_ble_gatts_characteristic_add(s_service_handle, &char_md, &attr_char_value, &s_config_handles);

    /* 3. Add Control Characteristic (Write Without Response) */
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write_wo_resp = 1;

    ble_uuid_t control_uuid = {
        .uuid = ULP_SVC_UUID_CONTROL,
        .type = s_uuid_type
    };

    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &control_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = 0;
    attr_char_value.max_len  = 16;
    attr_char_value.p_attr_md->vlen = 1;

    sd_ble_gatts_characteristic_add(s_service_handle, &char_md, &attr_char_value, &s_control_handles);

    /* 4. Add Status Characteristic (Read, 24 bytes) */
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;

    ble_uuid_t status_uuid = {
        .uuid = ULP_SVC_UUID_STATUS,
        .type = s_uuid_type
    };

    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &status_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = sizeof(status_payload_t);
    attr_char_value.max_len  = sizeof(status_payload_t);
    attr_char_value.p_value  = s_status_buffer;

    sd_ble_gatts_characteristic_add(s_service_handle, &char_md, &attr_char_value, &s_status_handles);

    return NRF_SUCCESS;
}

ret_code_t ble_ulp_svc_notify_telemetry(uint16_t conn_handle, const uint8_t * p_data, uint16_t len)
{
    if (conn_handle == BLE_CONN_HANDLE_INVALID) {
        return NRF_ERROR_INVALID_STATE;
    }
    if (!s_telemetry_notify_enabled) {
        return NRF_ERROR_INVALID_STATE;
    }

    uint16_t hvx_len = len;
    ble_gatts_hvx_params_t hvx_params = {
        .handle = s_telemetry_handles.value_handle,
        .type   = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_len  = &hvx_len,
        .p_data = p_data
    };

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

bool ble_ulp_svc_is_telemetry_notify_enabled(void)
{
    return s_telemetry_notify_enabled;
}

void ble_ulp_svc_on_ble_evt(const ble_evt_t * p_ble_evt)
{
    if (!p_ble_evt) {
        return;
    }

    switch (p_ble_evt->evt_id) {
        case BLE_GAP_EVT_DISCONNECTED:
            s_telemetry_notify_enabled = false;
            break;

        case BLE_GATTS_EVT_WRITE: {
            const ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

            /* CCCD Write */
            if (p_evt_write->handle == s_telemetry_handles.cccd_handle) {
                if (p_evt_write->len == 2) {
                    s_telemetry_notify_enabled = (p_evt_write->p_data[0] & 0x01) != 0;
                }
            }
            /* Config Characteristic Write */
            else if (p_evt_write->handle == s_config_handles.value_handle) {
                config_payload_t new_cfg;
                if (protocol_decode_config(p_evt_write->p_data, p_evt_write->len, &new_cfg)) {
                    app_cfg_update(&new_cfg);
                } else {
                    diag_inc_bad_frames();
                }
            }
            /* Control Characteristic Write */
            else if (p_evt_write->handle == s_control_handles.value_handle) {
                uint8_t opcode = 0;
                const uint8_t * p_params = NULL;
                uint8_t param_len = 0;

                if (protocol_parse_control(p_evt_write->p_data, p_evt_write->len,
                                           &opcode, &p_params, &param_len))
                {
                    if (s_control_callback) {
                        s_control_callback(opcode, p_params, param_len);
                    }
                } else {
                    diag_inc_bad_frames();
                }
            }
            break;
        }

        default:
            break;
    }
}

void ble_ulp_svc_update_status(const status_payload_t * p_status)
{
    if (p_status) {
        memcpy(s_status_buffer, p_status, sizeof(status_payload_t));
    }
}
