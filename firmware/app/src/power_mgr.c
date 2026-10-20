/**
 * @file power_mgr.c
 * @brief Power Management Policy and System OFF Sequencer Implementation
 */

#include "power_mgr.h"
#include "retention.h"
#include "wake_mgr.h"
#include "hal_gpio_lp.h"
#include "hal_twi.h"
#include "hal_saadc.h"
#include "drv_lis2dw12.h"
#include "board_config.h"

ret_code_t power_mgr_init(void)
{
    ret_code_t err = nrf_pwr_mgmt_init();
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) {
        return err;
    }

#if defined(BOARD_HAS_DCDC_INDUCTORS) && (BOARD_HAS_DCDC_INDUCTORS == 1)
    /* Enable DC/DC regulator for optimal radio current draw */
    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
#endif

    /* Configure lowest power CPU sleep mode */
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

    return NRF_SUCCESS;
}

void power_mgr_run_sleep(void)
{
    nrf_pwr_mgmt_run();
}

ret_code_t power_mgr_enter_system_off(uint8_t wake_mask)
{
    /* 1. Guard check: if accelerometer interrupt is latched, clear it first */
    if (wake_mask & WAKE_MASK_MOTION) {
        drv_lis2dw12_ack_wakeup();
    }

    /* 2. Guard check: ensure no wake pin is currently held asserted (Flowchart Step H) */
    if (wake_mgr_is_any_source_asserted(wake_mask)) {
        /* A wake pin (e.g. Button held down) is actively at its sense level.
         * Defer System OFF entry to prevent an immediate wake-up loop defect.
         */
        return NRF_ERROR_BUSY;
    }

    /* 3. Persist volatile state into retained RAM record and GPREGRET */
    retention_save_before_sysoff();

    /* 4. Quiesce all peripherals */
    hal_twi_force_disable();
    hal_saadc_force_uninit();

    /* 5. Park non-wake GPIO and arm wake sources with SENSE */
    wake_mgr_arm_sources(wake_mask);

    /* 6. Enter System OFF deep sleep. Execution halts until external wake reset. */
    uint32_t err = sd_power_system_off();
    APP_ERROR_CHECK(err);

    /* Fallback spin loop for emulated System OFF under hardware debugger */
    for (;;) {
#if defined(__GNUC__) && !defined(HOST_TEST)
        __asm volatile("wfe");
#endif
    }

    return NRF_SUCCESS;
}
