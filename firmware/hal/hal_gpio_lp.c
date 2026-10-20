/**
 * @file hal_gpio_lp.c
 * @brief Low-power GPIO Hardware Abstraction Layer Implementation
 */

#include "hal_gpio_lp.h"

/* Wake mask constants matching ULPBLE-DOC-02 */
#define WAKE_MASK_BUTTON    (1u << 0)
#define WAKE_MASK_MOTION    (1u << 1)
#define WAKE_MASK_EXT_RTC   (1u << 2)

static bool is_board_allocated_pin(uint32_t pin)
{
    if (pin == PIN_I2C_SDA || pin == PIN_I2C_SCL ||
        pin == PIN_BTN     || pin == PIN_ACC_INT1 ||
        pin == PIN_TEMP_ALERT || pin == PIN_RTC_INT)
    {
        return true;
    }
#if defined(DEBUG) || (defined(LOG) && (LOG == 1))
    if (pin == PIN_LED || pin == PIN_UART_TX || pin == PIN_UART_RX) {
        return true;
    }
#endif
    return false;
}

void hal_gpio_lp_init_all_unused(void)
{
    /* Iterate Port 0 pins (0 to 31) */
    for (uint32_t pin = 0; pin < 32; pin++) {
        if (!is_board_allocated_pin(pin)) {
            nrf_gpio_cfg(pin,
                         NRF_GPIO_PIN_DIR_INPUT,
                         NRF_GPIO_PIN_INPUT_DISCONNECT,
                         NRF_GPIO_PIN_NOPULL,
                         NRF_GPIO_PIN_S0S1,
                         NRF_GPIO_PIN_NOSENSE);
        }
    }

#if defined(BOARD_TARGET_NRF52840)
    /* nRF52840 has Port 1 pins (P1.00 - P1.15) */
    for (uint32_t pin = 32; pin < 48; pin++) {
        nrf_gpio_cfg(pin,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_NOSENSE);
    }
#endif

    /* Configure default state for allocated input pins */
    nrf_gpio_cfg(PIN_BTN,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_ACC_INT1,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLDOWN,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

#if defined(DEBUG) || (defined(LOG) && (LOG == 1))
    nrf_gpio_cfg(PIN_LED,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(PIN_LED);
#else
    /* Production build: disconnect LED pin entirely */
    nrf_gpio_cfg(PIN_LED,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);
#endif
}

void hal_gpio_lp_prepare_sysoff(uint8_t wake_mask)
{
    /* Disconnect I2C and debug pins */
    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_I2C_SCL,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_LED,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_UART_TX,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_UART_RX,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    /* Arm Button wake if requested */
    if (wake_mask & WAKE_MASK_BUTTON) {
        nrf_gpio_cfg_sense_input(PIN_BTN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    } else {
        nrf_gpio_cfg(PIN_BTN,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_NOSENSE);
    }

    /* Arm Accelerometer Motion wake if requested */
    if (wake_mask & WAKE_MASK_MOTION) {
        nrf_gpio_cfg_sense_input(PIN_ACC_INT1, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
    } else {
        nrf_gpio_cfg(PIN_ACC_INT1,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_NOSENSE);
    }

    /* Arm External RTC wake if requested */
    if (wake_mask & WAKE_MASK_EXT_RTC) {
        nrf_gpio_cfg_sense_input(PIN_RTC_INT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
    } else {
        nrf_gpio_cfg(PIN_RTC_INT,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_NOSENSE);
    }
}

bool hal_gpio_lp_is_pin_asserted(uint32_t pin, uint8_t active_level)
{
    /* Temporarily ensure input buffer connected to read level */
    uint32_t pin_val = nrf_gpio_pin_read(pin);
    return ((pin_val != 0) == (active_level != 0));
}

void hal_gpio_lp_led_set(bool enable)
{
#if defined(DEBUG) || (defined(LOG) && (LOG == 1))
    if (enable) {
        nrf_gpio_pin_set(PIN_LED);
    } else {
        nrf_gpio_pin_clear(PIN_LED);
    }
#else
    UNUSED_PARAMETER(enable);
#endif
}
