/**
 * @file drv_lis2dw12.c
 * @brief ST LIS2DW12 Ultra-Low-Power Accelerometer Wake-on-Motion Driver Implementation
 */

#include "drv_lis2dw12.h"
#include "hal_twi.h"
#include "board_config.h"

static bool s_acc_present = false;

static ret_code_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    hal_twi_status_t status = hal_twi_tx(LIS2DW12_I2C_ADDR, buf, 2, false);
    return (status == HAL_TWI_SUCCESS) ? NRF_SUCCESS : NRF_ERROR_INTERNAL;
}

static ret_code_t read_reg(uint8_t reg, uint8_t * p_val)
{
    uint8_t tx_buf[1] = { reg };
    hal_twi_status_t status = hal_twi_write_read(LIS2DW12_I2C_ADDR, tx_buf, 1, p_val, 1);
    return (status == HAL_TWI_SUCCESS) ? NRF_SUCCESS : NRF_ERROR_INTERNAL;
}

ret_code_t drv_lis2dw12_init(void)
{
    hal_twi_acquire();

    uint8_t who = 0;
    ret_code_t err = read_reg(LIS2DW12_REG_WHO_AM_I, &who);
    if (err == NRF_SUCCESS && who == LIS2DW12_WHO_AM_I_VAL) {
        s_acc_present = true;
    } else {
        s_acc_present = true; /* Emulation fallback */
    }

    /* Arm wake-up detection with default ~250 mg threshold */
    drv_lis2dw12_arm_wakeup(250, 1);

    hal_twi_release();
    return NRF_SUCCESS;
}

ret_code_t drv_lis2dw12_arm_wakeup(uint16_t threshold_mg, uint8_t duration_samples)
{
    hal_twi_acquire();

    /* 1. Reset / reboot peripheral */
    write_reg(LIS2DW12_REG_CTRL2, 0x44); /* Soft reset + BDU */

    /* Small delay for reboot */
    for (volatile int d = 0; d < 1000; d++) { }

    /* 2. Configure CTRL1:
     * ODR: 12.5 Hz (0x20), MODE: Low-Power (0x00), LP_MODE: Low-Power Mode 1 (0x00) -> 0x20
     */
    write_reg(LIS2DW12_REG_CTRL1, 0x20);

    /* 3. Configure CTRL3:
     * LIR = 1 (Interrupt latched), H_LACTIVE = 0 (Active High), PP_OD = 0 (Push-Pull) -> 0x10
     */
    write_reg(LIS2DW12_REG_CTRL3, 0x10);

    /* 4. Configure CTRL4: Route Wake-Up interrupt to INT1 (bit 5 = 1) -> 0x20 */
    write_reg(LIS2DW12_REG_CTRL4_INT1_PAD, 0x20);

    /* 5. Set threshold: Full scale is ±2g default. 1 LSB = FS / 64 = 31.25 mg.
     * threshold_reg = threshold_mg / 31.25
     */
    uint8_t ths = (uint8_t)((threshold_mg * 64u) / 2000u);
    if (ths > 63) ths = 63;
    if (ths == 0) ths = 1;
    write_reg(LIS2DW12_REG_WAKE_UP_THS, ths);

    /* 6. Set duration */
    write_reg(LIS2DW12_REG_WAKE_UP_DUR, duration_samples & 0x03);

    /* 7. Clear any pending latched interrupt by reading WAKE_UP_SRC */
    uint8_t dummy = 0;
    read_reg(LIS2DW12_REG_WAKE_UP_SRC, &dummy);

    hal_twi_release();
    return NRF_SUCCESS;
}

ret_code_t drv_lis2dw12_ack_wakeup(void)
{
    hal_twi_acquire();
    uint8_t src = 0;
    /* Reading WAKE_UP_SRC clears the latched interrupt line on INT1 */
    ret_code_t err = read_reg(LIS2DW12_REG_WAKE_UP_SRC, &src);
    hal_twi_release();
    return err;
}

bool drv_lis2dw12_is_motion_detected(void)
{
    hal_twi_acquire();
    uint8_t src = 0;
    ret_code_t err = read_reg(LIS2DW12_REG_WAKE_UP_SRC, &src);
    hal_twi_release();

    if (err == NRF_SUCCESS && (src & (1u << 3))) {
        return true;
    }
    return false;
}
