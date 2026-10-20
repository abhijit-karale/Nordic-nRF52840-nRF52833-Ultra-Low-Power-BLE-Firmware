/**
 * @file hal_twi.h
 * @brief Power-Gated I2C / TWIM Hardware Abstraction Layer
 *
 * Implements ephemeral peripheral enable/disable (acquire/release) to prevent
 * peripheral idle current drain, along with 9-pulse bus-recovery.
 */

#ifndef HAL_TWI_H_
#define HAL_TWI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sdk_common.h"
#include "board_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_TWI_SUCCESS = 0,
    HAL_TWI_ERR_BUSY,
    HAL_TWI_ERR_NACK,
    HAL_TWI_ERR_TIMEOUT,
    HAL_TWI_ERR_BUS_STUCK
} hal_twi_status_t;

/**
 * @brief Early initialization of TWI HAL
 */
void hal_twi_init(void);

/**
 * @brief Acquire I2C bus: enables TWIM peripheral and configures GPIOs.
 * Must be paired with hal_twi_release().
 */
hal_twi_status_t hal_twi_acquire(void);

/**
 * @brief Release I2C bus: disables TWIM peripheral and parks pins.
 */
void hal_twi_release(void);

/**
 * @brief Perform I2C Write transaction.
 *
 * @param addr 7-bit device address.
 * @param p_data Pointer to data buffer.
 * @param length Number of bytes to write.
 * @param no_stop If true, generate repeated START.
 */
hal_twi_status_t hal_twi_tx(uint8_t addr, const uint8_t * p_data, size_t length, bool no_stop);

/**
 * @brief Perform I2C Read transaction.
 *
 * @param addr 7-bit device address.
 * @param p_data Pointer to receive buffer.
 * @param length Number of bytes to read.
 */
hal_twi_status_t hal_twi_rx(uint8_t addr, uint8_t * p_data, size_t length);

/**
 * @brief Write then Read (register read pattern) with retry and bus recovery.
 */
hal_twi_status_t hal_twi_write_read(uint8_t addr,
                                   const uint8_t * p_tx_data, size_t tx_len,
                                   uint8_t * p_rx_data, size_t rx_len);

/**
 * @brief Perform standard 9-clock-pulse bus recovery for stuck slaves.
 */
void hal_twi_bus_recover(void);

/**
 * @brief Force immediate disable of TWIM peripheral (used before System OFF).
 */
void hal_twi_force_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TWI_H_ */
