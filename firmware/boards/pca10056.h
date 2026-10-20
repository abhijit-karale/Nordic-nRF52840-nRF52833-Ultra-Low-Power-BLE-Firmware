/**
 * @file pca10056.h
 * @brief Board Support Definitions for Nordic nRF52840-DK (PCA10056)
 */

#ifndef PCA10056_H_
#define PCA10056_H_

#define BOARD_NAME                  "PCA10056"
#define BOARD_TARGET_NRF52840       1
#define BOARD_FLASH_SIZE_KB         1024
#define BOARD_RAM_SIZE_KB           256

/* DK Buttons (active-low to GND) */
#define PIN_BTN                     11  /* Button 1 = P0.11 */
#define PIN_BTN2                    12  /* Button 2 = P0.12 */

/* DK LEDs (active-low) */
#define PIN_LED                     13  /* LED 1 = P0.13 */
#define PIN_LED2                    14  /* LED 2 = P0.14 */

/* DK Arduino Header I2C Pins */
#define PIN_I2C_SDA                 26  /* P0.26 / ARDUINO SDA */
#define PIN_I2C_SCL                 27  /* P0.27 / ARDUINO SCL */

/* Simulated / Header wake pins */
#define PIN_ACC_INT1                28  /* P0.28 */
#define PIN_TEMP_ALERT              29  /* P0.29 */
#define PIN_RTC_INT                 30  /* P0.30 */

/* DK UART Pins */
#define PIN_UART_TX                 6   /* P0.06 */
#define PIN_UART_RX                 8   /* P0.08 */

#define BTN_ACTIVE_LEVEL            0
#define ACC_INT1_ACTIVE_LEVEL       1
#define RTC_INT_ACTIVE_LEVEL        0

#define TMP117_I2C_ADDR             0x48
#define LIS2DW12_I2C_ADDR           0x19
#define RV3028_I2C_ADDR             0x52

#define BOARD_HAS_DCDC_INDUCTORS    1
#define BOARD_HAS_LFXO_CRYSTAL      1

#endif /* PCA10056_H_ */
