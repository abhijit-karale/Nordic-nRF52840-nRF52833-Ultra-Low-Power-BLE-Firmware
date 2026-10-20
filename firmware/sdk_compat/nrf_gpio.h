/**
 * @file nrf_gpio.h
 * @brief GPIO abstraction conforming to Nordic nRF5 SDK nrfx_gpio / nrf_gpio
 */

#ifndef NRF_GPIO_H__
#define NRF_GPIO_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NRF_GPIO_PIN_DIR_INPUT  = 0,
    NRF_GPIO_PIN_DIR_OUTPUT = 1
} nrf_gpio_pin_dir_t;

typedef enum {
    NRF_GPIO_PIN_INPUT_CONNECT    = 0,
    NRF_GPIO_PIN_INPUT_DISCONNECT = 1
} nrf_gpio_pin_input_t;

typedef enum {
    NRF_GPIO_PIN_NOPULL   = 0,
    NRF_GPIO_PIN_PULLDOWN = 1,
    NRF_GPIO_PIN_PULLUP   = 3
} nrf_gpio_pin_pull_t;

typedef enum {
    NRF_GPIO_PIN_S0S1 = 0,
    NRF_GPIO_PIN_H0S1 = 1,
    NRF_GPIO_PIN_S0H1 = 2,
    NRF_GPIO_PIN_H0H1 = 3,
    NRF_GPIO_PIN_D0S1 = 4,
    NRF_GPIO_PIN_D0H1 = 5,
    NRF_GPIO_PIN_S0D1 = 6,
    NRF_GPIO_PIN_H0D1 = 7
} nrf_gpio_pin_drive_t;

typedef enum {
    NRF_GPIO_PIN_NOSENSE    = 0,
    NRF_GPIO_PIN_SENSE_LOW  = 2,
    NRF_GPIO_PIN_SENSE_HIGH = 3
} nrf_gpio_pin_sense_t;

/* Pin manipulation functions/prototypes */
void nrf_gpio_cfg(uint32_t pin_number,
                  nrf_gpio_pin_dir_t dir,
                  nrf_gpio_pin_input_t input,
                  nrf_gpio_pin_pull_t pull,
                  nrf_gpio_pin_drive_t drive,
                  nrf_gpio_pin_sense_t sense);

void nrf_gpio_cfg_default(uint32_t pin_number);
void nrf_gpio_cfg_sense_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config, nrf_gpio_pin_sense_t sense_config);
void nrf_gpio_pin_set(uint32_t pin_number);
void nrf_gpio_pin_clear(uint32_t pin_number);
void nrf_gpio_pin_toggle(uint32_t pin_number);
void nrf_gpio_pin_write(uint32_t pin_number, uint32_t value);
uint32_t nrf_gpio_pin_read(uint32_t pin_number);

#endif /* NRF_GPIO_H__ */
