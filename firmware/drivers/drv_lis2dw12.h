/**
 * @file drv_lis2dw12.h
 * @brief ST LIS2DW12 Ultra-Low-Power 3-Axis Accelerometer Wake-on-Motion Driver
 *
 * Implements low-power wake-up detection (~380 nA mode) with latched interrupt on INT1
 * and interrupt release/acknowledge mechanism.
 */

#ifndef DRV_LIS2DW12_H_
#define DRV_LIS2DW12_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIS2DW12_REG_WHO_AM_I           0x0F
#define LIS2DW12_REG_CTRL1              0x20
#define LIS2DW12_REG_CTRL2              0x21
#define LIS2DW12_REG_CTRL3              0x22
#define LIS2DW12_REG_CTRL4_INT1_PAD     0x23
#define LIS2DW12_REG_CTRL5_INT2_PAD     0x24
#define LIS2DW12_REG_CTRL6              0x25
#define LIS2DW12_REG_STATUS             0x27
#define LIS2DW12_REG_OUT_X_L            0x28
#define LIS2DW12_REG_WAKE_UP_THS        0x34
#define LIS2DW12_REG_WAKE_UP_DUR        0x35
#define LIS2DW12_REG_WAKE_UP_SRC        0x38

#define LIS2DW12_WHO_AM_I_VAL           0x44

/**
 * @brief Initialize LIS2DW12 and verify communications.
 */
ret_code_t drv_lis2dw12_init(void);

/**
 * @brief Configure and arm wake-on-motion interrupt.
 *
 * Configures low-power mode, wake-up threshold, latched mode on INT1 (active-high).
 *
 * @param threshold_mg Wake threshold in milli-g (e.g. 250 mg).
 * @param duration_samples Duration threshold in samples.
 */
ret_code_t drv_lis2dw12_arm_wakeup(uint16_t threshold_mg, uint8_t duration_samples);

/**
 * @brief Acknowledge and clear latched wake interrupt by reading WAKE_UP_SRC.
 * Essential before entering System OFF to prevent immediate re-wake.
 */
ret_code_t drv_lis2dw12_ack_wakeup(void);

/**
 * @brief Check if a motion event triggered the interrupt.
 */
bool drv_lis2dw12_is_motion_detected(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_LIS2DW12_H_ */
