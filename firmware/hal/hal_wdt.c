/**
 * @file hal_wdt.c
 * @brief Watchdog Timer and Multi-Source Health Monitor HAL Implementation
 */

#include "hal_wdt.h"

typedef struct {
    volatile uint32_t TASKS_START;
    volatile uint32_t RESERVED0[63];
    volatile uint32_t EVENTS_TIMEOUT;
    volatile uint32_t RESERVED1[63];
    volatile uint32_t INTENSET;
    volatile uint32_t INTENCLR;
    volatile uint32_t RESERVED2[61];
    volatile uint32_t RUNSTATUS;
    volatile uint32_t REQSTATUS;
    volatile uint32_t RESERVED3[62];
    volatile uint32_t CRV;
    volatile uint32_t RREN;
    volatile uint32_t CONFIG;
    volatile uint32_t RESERVED4[60];
    volatile uint32_t RR[8];
} NRF_WDT_Type;

static NRF_WDT_Type s_mock_wdt;
#ifndef NRF_WDT
#define NRF_WDT (&s_mock_wdt)
#endif

#define NRF_WDT_RR_VALUE 0x6E524635 /* 'nRF5' reload key */

static volatile uint8_t s_health_accumulated_mask = 0;
static bool s_wdt_initialized = false;

ret_code_t hal_wdt_init(uint32_t timeout_ms)
{
    if (timeout_ms == 0) {
        timeout_ms = 10000; /* 10 s default */
    }

    /* Configure WDT to continue running while CPU is in System ON sleep (bit 0 = 1) */
    NRF_WDT->CONFIG = 0x01;

    /* CRV = (timeout_ms * 32768) / 1000 */
    NRF_WDT->CRV = (uint32_t)(((uint64_t)timeout_ms * 32768ULL) / 1000ULL);

    /* Enable reload request register 0 */
    NRF_WDT->RREN = 0x01;

    /* Start watchdog */
    NRF_WDT->TASKS_START = 1;

    s_health_accumulated_mask = HEALTH_MASK_ALL;
    s_wdt_initialized = true;

    /* First initial feed */
    NRF_WDT->RR[0] = NRF_WDT_RR_VALUE;

    return NRF_SUCCESS;
}

void hal_wdt_feed_health_bit(uint8_t health_bit)
{
    s_health_accumulated_mask |= health_bit;
}

bool hal_wdt_process_health_check(void)
{
    if (!s_wdt_initialized) {
        return false;
    }

    /* Verify all health bits (Main, BLE, Timer) were reported since last check */
    if ((s_health_accumulated_mask & HEALTH_MASK_ALL) == HEALTH_MASK_ALL) {
        /* Reload hardware watchdog */
        NRF_WDT->RR[0] = NRF_WDT_RR_VALUE;
        /* Reset mask for the next inspection period */
        s_health_accumulated_mask = 0;
        return true;
    }

    /* Do not feed if any subsystem stalled - allows WDT to reset safely */
    return false;
}
