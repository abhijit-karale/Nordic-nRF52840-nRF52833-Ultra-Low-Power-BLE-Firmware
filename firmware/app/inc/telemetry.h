/**
 * @file telemetry.h
 * @brief Telemetry Sampling Orchestrator and Payload Builder
 *
 * Coordinates sensor acquisition (TMP117 one-shot, SAADC VDD measurement),
 * packs state into 20-byte Telemetry frame, and transmits via BLE GATT notification.
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize telemetry subsystem.
 */
void telemetry_init(void);

/**
 * @brief Trigger sensor measurement and assemble the 20-byte telemetry frame.
 *
 * @param pwr_state Current power state (0:ADV_FAST, 1:ADV_SLOW, 2:CONN_IDLE, 3:CONN_ACTIVE).
 * @param[out] p_frame_out Output telemetry struct.
 * @return ret_code_t NRF_SUCCESS on successful build.
 */
ret_code_t telemetry_sample_and_build(uint8_t pwr_state, telemetry_frame_t * p_frame_out);

/**
 * @brief Transmit telemetry notification over BLE if connected and notifications enabled.
 *
 * @param conn_handle Active BLE connection handle.
 * @return ret_code_t NRF_SUCCESS if notified.
 */
ret_code_t telemetry_send_notification(uint16_t conn_handle);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_H_ */
