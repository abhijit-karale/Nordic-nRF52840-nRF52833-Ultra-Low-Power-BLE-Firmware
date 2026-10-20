/**
 * @file drv_tmp117.h
 * @brief TI TMP117 Low-Power Digital Temperature Sensor Driver
 *
 * Implements one-shot measurement mode with immediate return to shutdown,
 * yielding zero static current when not measuring.
 */

#ifndef DRV_TMP117_H_
#define DRV_TMP117_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TMP117_REG_TEMP         0x00
#define TMP117_REG_CONFIG       0x01
#define TMP117_REG_THIGH        0x02
#define TMP117_REG_TLOW         0x03
#define TMP117_REG_EEPROM_UL    0x04
#define TMP117_REG_EEPROM1      0x05
#define TMP117_REG_EEPROM2      0x06
#define TMP117_REG_TEMP_OFFSET  0x07
#define TMP117_REG_EEPROM3      0x08
#define TMP117_REG_DEVICE_ID    0x0F

#define TMP117_DEVICE_ID_VALUE  0x0117

#define TMP117_MOD_CONTINUOUS   (0x00u << 10)
#define TMP117_MOD_SHUTDOWN     (0x01u << 10)
#define TMP117_MOD_ONESHOT      (0x03u << 10)
#define TMP117_FLAG_DATA_READY  (0x01u << 13)

/**
 * @brief Initialize TMP117 sensor and put into low-power shutdown.
 *
 * @return ret_code_t NRF_SUCCESS on successful probe and shutdown.
 */
ret_code_t drv_tmp117_init(void);

/**
 * @brief Perform a one-shot temperature measurement.
 *
 * Wakes sensor, initiates conversion, reads temperature register,
 * puts sensor back to shutdown, and converts raw reading to centi-degrees Celsius.
 *
 * @param[out] p_temp_c_x100 Output in centi-degrees Celsius (e.g. 2540 = 25.40 °C).
 * @return ret_code_t NRF_SUCCESS on valid reading.
 */
ret_code_t drv_tmp117_sample_temp_c_x100(int16_t * p_temp_c_x100);

/**
 * @brief Put sensor into explicit shutdown mode.
 */
ret_code_t drv_tmp117_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_TMP117_H_ */
