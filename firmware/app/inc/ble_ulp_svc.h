/**
 * @file ble_ulp_svc.h
 * @brief ULP Custom GATT Service Implementation
 *
 * Implements 128-bit vendor-specific ULP service:
 * - Telemetry: UUID 0x0002, Notify (20 bytes)
 * - Config:    UUID 0x0003, Read/Write (15 bytes)
 * - Control:   UUID 0x0004, Write Without Response (Opcode framing)
 * - Status:    UUID 0x0005, Read (24 bytes)
 */

#ifndef BLE_ULP_SVC_H_
#define BLE_ULP_SVC_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ULP_SVC_UUID_BASE { 0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, \
                            0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00 }

#define ULP_SVC_UUID_SERVICE        0x0001
#define ULP_SVC_UUID_TELEMETRY      0x0002
#define ULP_SVC_UUID_CONFIG         0x0003
#define ULP_SVC_UUID_CONTROL        0x0004
#define ULP_SVC_UUID_STATUS         0x0005

typedef void (*ble_ulp_control_handler_t)(uint8_t opcode, const uint8_t * p_params, uint8_t len);

/**
 * @brief Initialize custom ULP GATT service and register characteristics with SoftDevice.
 *
 * @param control_handler Callback function invoked on valid Control characteristic writes.
 * @return ret_code_t NRF_SUCCESS on successful registration.
 */
ret_code_t ble_ulp_svc_init(ble_ulp_control_handler_t control_handler);

/**
 * @brief Send 20-byte Telemetry notification to connected central.
 *
 * @param conn_handle Connection handle.
 * @param p_data Pointer to 20-byte payload.
 * @param len Length (must be 20).
 * @return ret_code_t NRF_SUCCESS if enqueued, or NRF_ERROR_INVALID_STATE if CCCD not enabled.
 */
ret_code_t ble_ulp_svc_notify_telemetry(uint16_t conn_handle, const uint8_t * p_data, uint16_t len);

/**
 * @brief Check if connected central has enabled Telemetry notifications (CCCD = 1).
 */
bool ble_ulp_svc_is_telemetry_notify_enabled(void);

/**
 * @brief SoftDevice BLE event handler for GATT service.
 *
 * @param p_ble_evt SoftDevice event pointer.
 */
void ble_ulp_svc_on_ble_evt(const ble_evt_t * p_ble_evt);

/**
 * @brief Update Status characteristic payload.
 */
void ble_ulp_svc_update_status(const status_payload_t * p_status);

#ifdef __cplusplus
}
#endif

#endif /* BLE_ULP_SVC_H_ */
