/**
 * @file ble_gap_gatts.h
 * @brief BLE GAP and GATTS types and declarations matching Nordic S140 SoftDevice
 */

#ifndef BLE_GAP_GATTS_H__
#define BLE_GAP_GATTS_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"

#define BLE_UUID_TYPE_BLE                       0x01
#define BLE_UUID_TYPE_VENDOR_BEGIN              0x02

#define BLE_GATT_CPF_FORMAT_UINT8               0x04
#define BLE_GATT_CPF_FORMAT_UINT16              0x06
#define BLE_GATT_CPF_FORMAT_SINT16              0x0E

#define BLE_GATT_STATUS_SUCCESS                 0x0000

#define BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE 0x06
#define BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE 0x05

#define BLE_CONN_HANDLE_INVALID                 0xFFFF

typedef struct {
    uint16_t uuid;
    uint8_t  type;
} ble_uuid_t;

typedef struct {
    uint8_t uuid128[16];
} ble_uuid128_t;

typedef struct {
    uint8_t broadcast       : 1;
    uint8_t read            : 1;
    uint8_t write_wo_resp   : 1;
    uint8_t write           : 1;
    uint8_t notify          : 1;
    uint8_t indicate        : 1;
    uint8_t auth_signed_wr  : 1;
} ble_gatt_char_props_t;

typedef struct {
    uint16_t service_handle;
    uint16_t value_handle;
    uint16_t user_desc_handle;
    uint16_t cccd_handle;
    uint16_t sccd_handle;
} ble_gatts_char_handles_t;

typedef struct {
    uint8_t sm : 4;
    uint8_t lv : 4;
} ble_gap_conn_sec_mode_t;

#define BLE_GAP_CONN_SEC_MODE_SET_OPEN(p_mode) \
    do { (p_mode)->sm = 1; (p_mode)->lv = 1; } while (0)

#define BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(p_mode) \
    do { (p_mode)->sm = 0; (p_mode)->lv = 0; } while (0)

typedef struct {
    ble_gap_conn_sec_mode_t read_perm;
    ble_gap_conn_sec_mode_t write_perm;
    uint8_t                 vlen : 1;
} ble_gatts_attr_md_t;

typedef struct {
    ble_uuid_t const      * p_uuid;
    ble_gatts_attr_md_t   * p_attr_md;
    uint16_t                init_len;
    uint16_t                init_offs;
    uint16_t                max_len;
    uint8_t               * p_value;
} ble_gatts_attr_t;

typedef struct {
    ble_gatt_char_props_t   char_props;
    ble_gatts_attr_md_t   * p_cccd_md;
    ble_gatts_attr_md_t   * p_sccd_md;
    ble_gatts_attr_md_t   * p_user_desc_md;
    ble_gatts_attr_t      * p_attr_char_value;
} ble_gatts_char_md_t;

typedef enum {
    BLE_GATTS_SRVC_TYPE_PRIMARY   = 0x01,
    BLE_GATTS_SRVC_TYPE_SECONDARY = 0x02
} ble_gatts_srvc_type_t;

typedef struct {
    uint16_t min_conn_interval;
    uint16_t max_conn_interval;
    uint16_t slave_latency;
    uint16_t conn_sup_timeout;
} ble_gap_conn_params_t;

typedef enum {
    BLE_GAP_EVT_CONNECTED = 0x10,
    BLE_GAP_EVT_DISCONNECTED = 0x11,
    BLE_GAP_EVT_CONN_PARAM_UPDATE = 0x12,
    BLE_GAP_EVT_TIMEOUT = 0x13
} ble_gap_evt_type_t;

typedef enum {
    BLE_GATTS_EVT_WRITE = 0x50,
    BLE_GATTS_EVT_HVN_TX_COMPLETE = 0x51
} ble_gatts_evt_type_t;

typedef struct {
    uint16_t handle;
    uint16_t offset;
    uint16_t len;
    uint8_t const * p_data;
} ble_gatts_evt_write_t;

typedef struct {
    uint16_t conn_handle;
    union {
        struct {
            ble_gap_conn_params_t conn_params;
        } connected;
        struct {
            uint8_t reason;
        } disconnected;
    } params;
} ble_gap_evt_t;

typedef struct {
    uint16_t conn_handle;
    union {
        ble_gatts_evt_write_t write;
    } params;
} ble_gatts_evt_t;

typedef struct {
    uint16_t evt_id;
    union {
        ble_gap_evt_t   gap_evt;
        ble_gatts_evt_t gatts_evt;
    } evt;
} ble_evt_t;

typedef struct {
    uint16_t handle;
    uint8_t  type;
    uint16_t offset;
    uint16_t * p_len;
    uint8_t const * p_data;
} ble_gatts_hvx_params_t;

#define BLE_GATT_HVX_NOTIFICATION 0x01

/* SoftDevice BLE prototypes */
ret_code_t sd_ble_uuid_vs_add(ble_uuid128_t const * p_vs_uuid, uint8_t * p_uuid_type);
ret_code_t sd_ble_gatts_service_add(uint8_t type, ble_uuid_t const * p_uuid, uint16_t * p_handle);
ret_code_t sd_ble_gatts_characteristic_add(uint16_t service_handle,
                                          ble_gatts_char_md_t const * p_char_md,
                                          ble_gatts_attr_t const * p_attr_char_value,
                                          ble_gatts_char_handles_t * p_handles);
ret_code_t sd_ble_gatts_hvx(uint16_t conn_handle, ble_gatts_hvx_params_t const * p_hvx_params);
ret_code_t sd_ble_gap_tx_power_set(uint8_t role, uint16_t handle, int8_t tx_power);
ret_code_t sd_ble_gap_adv_start(uint8_t adv_handle, uint8_t conn_cfg_tag);
ret_code_t sd_ble_gap_adv_stop(uint8_t adv_handle);
ret_code_t sd_ble_gap_disconnect(uint16_t conn_handle, uint8_t hci_status_code);

#endif /* BLE_GAP_GATTS_H__ */
