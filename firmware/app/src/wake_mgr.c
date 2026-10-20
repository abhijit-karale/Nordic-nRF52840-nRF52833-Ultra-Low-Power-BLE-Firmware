/**
 * @file wake_mgr.c
 * @brief Wake Source Configuration and Boot Reason Decoder Implementation
 */

#include "wake_mgr.h"
#include "hal_gpio_lp.h"
#include "board_config.h"

static wake_reason_t s_last_wake_reason = WAKE_POR;

wake_reason_t wake_mgr_capture_reset_reason(void)
{
    uint32_t r = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = 0xFFFFFFFFu; /* Clear reset reason bits */

    if (r & POWER_RESETREAS_OFF_Msk) {
        /* Woke up from System OFF via GPIO DETECT */
        uint32_t latch0 = NRF_P0->LATCH;
        NRF_P0->LATCH = latch0; /* Clear latches */

        if (latch0 & (1u << PIN_BTN)) {
            s_last_wake_reason = WAKE_BUTTON_EVT;
            return s_last_wake_reason;
        }
        if (latch0 & (1u << PIN_ACC_INT1)) {
            s_last_wake_reason = WAKE_MOTION_EVT;
            return s_last_wake_reason;
        }
        if (latch0 & (1u << PIN_RTC_INT)) {
            s_last_wake_reason = WAKE_RTC_EVT;
            return s_last_wake_reason;
        }

        s_last_wake_reason = WAKE_BUTTON_EVT; /* Fallback if latch masked */
        return s_last_wake_reason;
    }

    if (r & POWER_RESETREAS_DOG_Msk) {
        s_last_wake_reason = WAKE_WDT;
        return s_last_wake_reason;
    }
    if (r & POWER_RESETREAS_SREQ_Msk) {
        s_last_wake_reason = WAKE_SOFT;
        return s_last_wake_reason;
    }
    if (r & POWER_RESETREAS_LOCKUP_Msk) {
        s_last_wake_reason = WAKE_FAULT;
        return s_last_wake_reason;
    }

    s_last_wake_reason = WAKE_POR;
    return s_last_wake_reason;
}

wake_reason_t wake_mgr_get_last_reason(void)
{
    return s_last_wake_reason;
}

void wake_mgr_arm_sources(uint8_t wake_mask)
{
    hal_gpio_lp_prepare_sysoff(wake_mask);
}

bool wake_mgr_is_any_source_asserted(uint8_t wake_mask)
{
    if ((wake_mask & WAKE_MASK_BUTTON) &&
        hal_gpio_lp_is_pin_asserted(PIN_BTN, BTN_ACTIVE_LEVEL))
    {
        return true;
    }

    if ((wake_mask & WAKE_MASK_MOTION) &&
        hal_gpio_lp_is_pin_asserted(PIN_ACC_INT1, ACC_INT1_ACTIVE_LEVEL))
    {
        return true;
    }

    if ((wake_mask & WAKE_MASK_EXT_RTC) &&
        hal_gpio_lp_is_pin_asserted(PIN_RTC_INT, RTC_INT_ACTIVE_LEVEL))
    {
        return true;
    }

    return false;
}
