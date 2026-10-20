/**
 * @file wake_mgr.h
 * @brief Wake Source Configuration and Boot Reason Decoder
 *
 * Captures RESETREAS and GPIO LATCH registers before SoftDevice initialization,
 * categorizes wake source, and arms wake pins for System OFF deep sleep.
 */

#ifndef WAKE_MGR_H_
#define WAKE_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAKE_MASK_BUTTON    (1u << 0)
#define WAKE_MASK_MOTION    (1u << 1)
#define WAKE_MASK_EXT_RTC   (1u << 2)

typedef enum {
    WAKE_NONE = 0,
    WAKE_POR,
    WAKE_BUTTON_EVT,
    WAKE_MOTION_EVT,
    WAKE_RTC_EVT,
    WAKE_WDT,
    WAKE_SOFT,
    WAKE_FAULT
} wake_reason_t;

/**
 * @brief Read and clear RESETREAS and GPIO LATCH registers to determine boot reason.
 * MUST be executed at early boot before SoftDevice is initialized.
 *
 * @return wake_reason_t Decoded wake cause.
 */
wake_reason_t wake_mgr_capture_reset_reason(void);

/**
 * @brief Get the last captured wake reason.
 */
wake_reason_t wake_mgr_get_last_reason(void);

/**
 * @brief Arm configured GPIO wake pins for System OFF.
 *
 * @param wake_mask Bitmask of wake sources to enable.
 */
void wake_mgr_arm_sources(uint8_t wake_mask);

/**
 * @brief Check if any currently armed wake source is already asserted.
 * Guard against the immediate-wake loop defect.
 *
 * @param wake_mask Bitmask of wake sources.
 * @return true if any armed pin is asserted, false otherwise.
 */
bool wake_mgr_is_any_source_asserted(uint8_t wake_mask);

#ifdef __cplusplus
}
#endif

#endif /* WAKE_MGR_H_ */
