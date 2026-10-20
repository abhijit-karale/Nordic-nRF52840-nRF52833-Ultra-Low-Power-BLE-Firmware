/**
 * @file hal_saadc.h
 * @brief Power-Gated SAADC Hardware Abstraction Layer
 *
 * Direct VDD channel sampling with internal 0.6V reference and 1/6 gain,
 * completely uninitializing the SAADC between samples to consume zero quiescent current.
 */

#ifndef HAL_SAADC_H_
#define HAL_SAADC_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SAADC HAL
 */
void hal_saadc_init(void);

/**
 * @brief Perform a one-shot measurement of VDD in millivolts.
 * Initializes SAADC, configures VDD channel, samples, converts to mV,
 * and powers down the SAADC before returning.
 *
 * @param[out] p_vbat_mv Output variable for measured VDD in millivolts.
 * @return ret_code_t NRF_SUCCESS on success.
 */
ret_code_t hal_saadc_sample_vdd_mv(uint16_t * p_vbat_mv);

/**
 * @brief Force complete uninitialization and power down of SAADC (before System OFF).
 */
void hal_saadc_force_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SAADC_H_ */
