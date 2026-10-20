/**
 * @file ulp_board_52833.h
 * @brief Board Support Definitions for ULP-BLE Node on Nordic nRF52833
 *
 * Pin assignments and power characteristics for nRF52833 port target.
 */

#ifndef ULP_BOARD_52833_H_
#define ULP_BOARD_52833_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_NAME                  "ULP_BOARD_52833"
#define BOARD_TARGET_NRF52833       1
#define BOARD_FLASH_SIZE_KB         512
#define BOARD_RAM_SIZE_KB           128

/* I2C / TWIM0 Pins */
#define PIN_I2C_SDA                 26  /* P0.26 */
#define PIN_I2C_SCL                 27  /* P0.27 */

/* Wake / Input Pins */
#define PIN_BTN                     11  /* P0.11 */
#define PIN_ACC_INT1                28  /* P0.28 */
#define PIN_TEMP_ALERT              29  /* P0.29 */
#define PIN_RTC_INT                 30  /* P0.30 */

/* Debug Pins */
#define PIN_LED                     13  /* P0.13 */
#define PIN_UART_TX                 6   /* P0.06 */
#define PIN_UART_RX                 8   /* P0.08 */

/* Polarity & Sense Configuration */
#define BTN_ACTIVE_LEVEL            0
#define ACC_INT1_ACTIVE_LEVEL       1
#define RTC_INT_ACTIVE_LEVEL        0

/* Sensor I2C 7-bit Addresses */
#define TMP117_I2C_ADDR             0x48
#define LIS2DW12_I2C_ADDR           0x19
#define RV3028_I2C_ADDR             0x52

/* Board Power Capabilities */
#define BOARD_HAS_DCDC_INDUCTORS    1
#define BOARD_HAS_LFXO_CRYSTAL      1

#ifdef __cplusplus
}
#endif

#endif /* ULP_BOARD_52833_H_ */
