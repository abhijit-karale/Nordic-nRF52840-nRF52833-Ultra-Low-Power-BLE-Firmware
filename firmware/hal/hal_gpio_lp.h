/**
 * @file hal_gpio_lp.h
 * @brief Low-power GPIO Hardware Abstraction Layer
 *
 * Implements low-power pin discipline, unused pin parking (input disconnect),
 * and System OFF SENSE preparation and assertions.
 */

#ifndef HAL_GPIO_LP_H_
#define HAL_GPIO_LP_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"
#include "board_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all unused SoC GPIO pins into lowest power state
 * (Input, buffer disconnected, no pull).
 */
void hal_gpio_lp_init_all_unused(void);

/**
 * @brief Park non-essential pins and arm System OFF wake sources.
 *
 * @param wake_mask Bitmask of enabled wake sources.
 */
void hal_gpio_lp_prepare_sysoff(uint8_t wake_mask);

/**
 * @brief Check if a pin is currently held at its active/sense level.
 *
 * Used to avoid the immediate-wake loop defect when entering System OFF.
 *
 * @param pin Pin number.
 * @param active_level Expected active level (0 for low, 1 for high).
 * @return true if pin is currently asserted.
 */
bool hal_gpio_lp_is_pin_asserted(uint32_t pin, uint8_t active_level);

/**
 * @brief Set debug LED state (disabled in production low-power builds).
 *
 * @param enable true to turn on, false to turn off.
 */
void hal_gpio_lp_led_set(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_LP_H_ */
