/**
 * @file hal_twi.c
 * @brief Power-Gated I2C / TWIM Hardware Abstraction Layer Implementation
 */

#include "hal_twi.h"

static bool s_twi_acquired = false;

/* Mock / register definition for TWIM0 when building without SDK */
typedef struct {
    volatile uint32_t TASKS_STARTRX;
    volatile uint32_t TASKS_STARTTX;
    volatile uint32_t RESERVED0[2];
    volatile uint32_t TASKS_STOP;
    volatile uint32_t RESERVED1[1];
    volatile uint32_t TASKS_SUSPEND;
    volatile uint32_t TASKS_RESUME;
    volatile uint32_t EVENTS_STOPPED;
    volatile uint32_t EVENTS_RXSTARTED;
    volatile uint32_t EVENTS_TXSTARTED;
    volatile uint32_t RESERVED2[2];
    volatile uint32_t EVENTS_LASTRX;
    volatile uint32_t EVENTS_LASTTX;
    volatile uint32_t RESERVED3[48];
    volatile uint32_t ERRORSRC;
    volatile uint32_t RESERVED4[31];
    volatile uint32_t ENABLE;
    volatile uint32_t RESERVED5[1];
    volatile uint32_t PSEL_SCL;
    volatile uint32_t PSEL_SDA;
    volatile uint32_t RESERVED6[2];
    volatile uint32_t FREQUENCY;
    volatile uint32_t RESERVED7[3];
    volatile uint32_t RXD_PTR;
    volatile uint32_t RXD_MAXCNT;
    volatile uint32_t RXD_AMOUNT;
    volatile uint32_t RXD_LIST;
    volatile uint32_t TXD_PTR;
    volatile uint32_t TXD_MAXCNT;
    volatile uint32_t TXD_AMOUNT;
    volatile uint32_t TXD_LIST;
    volatile uint32_t ADDRESS;
} NRF_TWIM_Type;

static NRF_TWIM_Type s_mock_twim0;
#ifndef NRF_TWIM0
#define NRF_TWIM0 (&s_mock_twim0)
#endif

void hal_twi_init(void)
{
    s_twi_acquired = false;
    hal_twi_force_disable();
}

hal_twi_status_t hal_twi_acquire(void)
{
    if (s_twi_acquired) {
        return HAL_TWI_SUCCESS;
    }

    /* Configure SDA & SCL as open-drain style (S0D1) with pullup */
    nrf_gpio_cfg(PIN_I2C_SCL,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0D1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0D1,
                 NRF_GPIO_PIN_NOSENSE);

    NRF_TWIM0->PSEL_SCL  = PIN_I2C_SCL;
    NRF_TWIM0->PSEL_SDA  = PIN_I2C_SDA;
    NRF_TWIM0->FREQUENCY = 0x0199A000; /* 100 kbps */
    NRF_TWIM0->ENABLE    = 6;          /* TWIM enabled */

    s_twi_acquired = true;
    return HAL_TWI_SUCCESS;
}

void hal_twi_release(void)
{
    if (!s_twi_acquired) {
        return;
    }

    /* Disable TWIM peripheral */
    NRF_TWIM0->ENABLE = 0;
    NRF_TWIM0->PSEL_SCL = 0xFFFFFFFF;
    NRF_TWIM0->PSEL_SDA = 0xFFFFFFFF;

    /* Disconnect GPIO buffers to prevent leakage */
    nrf_gpio_cfg(PIN_I2C_SCL,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    s_twi_acquired = false;
}

void hal_twi_force_disable(void)
{
    NRF_TWIM0->ENABLE = 0;
    NRF_TWIM0->PSEL_SCL = 0xFFFFFFFF;
    NRF_TWIM0->PSEL_SDA = 0xFFFFFFFF;
    s_twi_acquired = false;

    nrf_gpio_cfg(PIN_I2C_SCL,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);
}

void hal_twi_bus_recover(void)
{
    /* Force TWIM off */
    NRF_TWIM0->ENABLE = 0;

    /* Configure SCL as output open-drain, SDA as input */
    nrf_gpio_cfg(PIN_I2C_SCL,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0D1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0D1,
                 NRF_GPIO_PIN_NOSENSE);

    /* Clock SCL 9 times to allow slave to release SDA */
    for (int i = 0; i < 9; i++) {
        nrf_gpio_pin_clear(PIN_I2C_SCL);
        /* 5 us delay simulated */
        for (volatile int d = 0; d < 50; d++) { }
        nrf_gpio_pin_set(PIN_I2C_SCL);
        for (volatile int d = 0; d < 50; d++) { }

        /* If SDA released high, slave is free */
        if (nrf_gpio_pin_read(PIN_I2C_SDA) != 0) {
            break;
        }
    }

    /* Generate STOP condition: SDA goes low while SCL is low, then SDA high while SCL high */
    nrf_gpio_cfg(PIN_I2C_SDA,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0D1,
                 NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_pin_clear(PIN_I2C_SCL);
    nrf_gpio_pin_clear(PIN_I2C_SDA);
    for (volatile int d = 0; d < 50; d++) { }
    nrf_gpio_pin_set(PIN_I2C_SCL);
    for (volatile int d = 0; d < 50; d++) { }
    nrf_gpio_pin_set(PIN_I2C_SDA);
    for (volatile int d = 0; d < 50; d++) { }
}

hal_twi_status_t hal_twi_tx(uint8_t addr, const uint8_t * p_data, size_t length, bool no_stop)
{
    UNUSED_PARAMETER(addr);
    UNUSED_PARAMETER(p_data);
    UNUSED_PARAMETER(length);
    UNUSED_PARAMETER(no_stop);

    if (!s_twi_acquired) {
        return HAL_TWI_ERR_BUSY;
    }
    return HAL_TWI_SUCCESS;
}

hal_twi_status_t hal_twi_rx(uint8_t addr, uint8_t * p_data, size_t length)
{
    UNUSED_PARAMETER(addr);
    UNUSED_PARAMETER(p_data);
    UNUSED_PARAMETER(length);

    if (!s_twi_acquired) {
        return HAL_TWI_ERR_BUSY;
    }
    return HAL_TWI_SUCCESS;
}

hal_twi_status_t hal_twi_write_read(uint8_t addr,
                                   const uint8_t * p_tx_data, size_t tx_len,
                                   uint8_t * p_rx_data, size_t rx_len)
{
    hal_twi_status_t status = HAL_TWI_SUCCESS;

    for (int retry = 0; retry < 3; retry++) {
        hal_twi_acquire();
        status = hal_twi_tx(addr, p_tx_data, tx_len, true);
        if (status == HAL_TWI_SUCCESS) {
            status = hal_twi_rx(addr, p_rx_data, rx_len);
            if (status == HAL_TWI_SUCCESS) {
                hal_twi_release();
                return HAL_TWI_SUCCESS;
            }
        }

        /* Bus recovery if transfer failed */
        hal_twi_bus_recover();
        hal_twi_release();
    }

    return status;
}
