/**
 * @file hal_wdt.h
 * @brief Watchdog Timer and Multi-Source Health Monitor HAL
 *
 * Implements hardware watchdog feeding gated by health flags from main loop,
 * BLE stack, and timer ticks.
 */

#ifndef HAL_WDT_H_
#define HAL_WDT_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HEALTH_BIT_MAIN     (1u << 0)
#define HEALTH_BIT_BLE      (1u << 1)
#define HEALTH_BIT_TIMER    (1u << 2)

#define HEALTH_MASK_ALL     (HEALTH_BIT_MAIN | HEALTH_BIT_BLE | HEALTH_BIT_TIMER)

/**
 * @brief Initialize hardware watchdog with 10 s timeout and start health monitoring timer.
 */
ret_code_t hal_wdt_init(uint32_t timeout_ms);

/**
 * @brief Report that a subsystem has completed work and is healthy.
 *
 * @param health_bit One of HEALTH_BIT_MAIN, HEALTH_BIT_BLE, HEALTH_BIT_TIMER.
 */
void hal_wdt_feed_health_bit(uint8_t health_bit);

/**
 * @brief Evaluate aggregated health mask and reload hardware watchdog if healthy.
 * Called periodically (every ~4 s) from app_timer callback.
 *
 * @return true if fed successfully, false if health criteria failed.
 */
bool hal_wdt_process_health_check(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_WDT_H_ */
