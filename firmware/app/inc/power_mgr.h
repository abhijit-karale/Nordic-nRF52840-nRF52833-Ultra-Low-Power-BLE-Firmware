/**
 * @file power_mgr.h
 * @brief Power Management Policy and System OFF Sequencer
 *
 * Implements low-power run loop, DC/DC converter enablement, peripheral quiescing,
 * retention persistence, wake pin guard checks, and System OFF entry.
 */

#ifndef POWER_MGR_H_
#define POWER_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize power management, enable DC/DC converter and low-power CPU mode.
 */
ret_code_t power_mgr_init(void);

/**
 * @brief Run one iteration of the power management sleep loop (System ON idle).
 * Calls WFE / sd_app_evt_wait().
 */
void power_mgr_run_sleep(void);

/**
 * @brief Safely transition into System OFF deep sleep.
 *
 * Steps:
 * 1. Checks if any wake source is currently asserted (prevents wake loop).
 * 2. Persists retention state (GPREGRET + .noinit CRC-16).
 * 3. Quiesces peripherals (TWIM, SAADC, UART).
 * 4. Parks unused GPIO and arms SENSE wake pins.
 * 5. Calls sd_power_system_off().
 *
 * @param wake_mask Bitmask of wake sources to enable.
 * @return ret_code_t NRF_ERROR_BUSY if a wake pin is actively asserted and entry was deferred.
 */
ret_code_t power_mgr_enter_system_off(uint8_t wake_mask);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MGR_H_ */
