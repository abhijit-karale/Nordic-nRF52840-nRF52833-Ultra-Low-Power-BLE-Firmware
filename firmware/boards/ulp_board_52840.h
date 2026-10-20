/**
 * @file ulp_board_52840.h
 * @brief Board Support Definitions for ULP-BLE Node on Nordic nRF52840
 *
 * Pin assignments and power characteristics as defined in ULPBLE-DOC-01.
 */

#ifndef ULP_BOARD_52840_H_
#define ULP_BOARD_52840_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_NAME                  "ULP_BOARD_52840"
#define BOARD_TARGET_NRF52840       1
#define BOARD_FLASH_SIZE_KB         1024
#define BOARD_RAM_SIZE_KB           256

/* I2C / TWIM0 Pins */
#define PIN_I2C_SDA                 26  /* P0.26 */
#define PIN_I2C_SCL                 27  /* P0.27 */

/* Wake / Input Pins */
#define PIN_BTN                     11  /* P0.11, active-low, internal pull-up */
#define PIN_ACC_INT1                28  /* P0.28, active-high, push-pull / pull-down */
#define PIN_TEMP_ALERT              29  /* P0.29, active-low alert */
#define PIN_RTC_INT                 30  /* P0.30, active-low, open-drain alarm */

/* Debug Pins (Disabled in production low-power profile) */
#define PIN_LED                     13  /* P0.13 */
#define PIN_UART_TX                 6   /* P0.06 */
#define PIN_UART_RX                 8   /* P0.08 */

/* Polarity & Sense Configuration */
#define BTN_ACTIVE_LEVEL            0   /* Active Low */
#define ACC_INT1_ACTIVE_LEVEL       1   /* Active High */
#define RTC_INT_ACTIVE_LEVEL        0   /* Active Low */

/* Sensor I2C 7-bit Addresses */
#define TMP117_I2C_ADDR             0x48
#define LIS2DW12_I2C_ADDR           0x19 /* Alternate: 0x18 depending on SA0 */
#define RV3028_I2C_ADDR             0x52

/* Board Power Capabilities */
#define BOARD_HAS_DCDC_INDUCTORS    1   /* DC/DC supported on board */
#define BOARD_HAS_LFXO_CRYSTAL      1   /* 32.768 kHz external crystal fitted */

#ifdef __cplusplus
}
#endif

#endif /* ULP_BOARD_52840_H_ */
