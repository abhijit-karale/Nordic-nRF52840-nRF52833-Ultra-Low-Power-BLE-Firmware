/**
 * @file drv_vbat.h
 * @brief Battery Voltage Monitoring and Health Policy Driver
 *
 * Implements moving-average filtered battery voltage measurements via SAADC
 * and evaluates low-battery / critical thresholds.
 */

#ifndef DRV_VBAT_H_
#define DRV_VBAT_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VBAT_WARNING_THRESHOLD_MV   2400  /* 2.4 V */
#define VBAT_CRITICAL_THRESHOLD_MV  2200  /* 2.2 V */

typedef enum {
    VBAT_LEVEL_OK = 0,
    VBAT_LEVEL_WARNING,
    VBAT_LEVEL_CRITICAL
} vbat_status_level_t;

/**
 * @brief Initialize battery measurement subsystem and populate initial average.
 */
ret_code_t drv_vbat_init(void);

/**
 * @brief Sample battery voltage and update 4-sample moving average filter.
 *
 * @param[out] p_vbat_mv Filtered battery voltage in millivolts.
 * @return ret_code_t NRF_SUCCESS on successful measurement.
 */
ret_code_t drv_vbat_sample_mv(uint16_t * p_vbat_mv);

/**
 * @brief Get the last evaluated battery status level (OK, WARNING, CRITICAL).
 */
vbat_status_level_t drv_vbat_get_level(void);

/**
 * @brief Check if battery has crossed the critical shutdown threshold.
 */
bool drv_vbat_is_critical(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_VBAT_H_ */
