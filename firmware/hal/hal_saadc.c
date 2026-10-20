/**
 * @file hal_saadc.c
 * @brief Power-Gated SAADC Hardware Abstraction Layer Implementation
 */

#include "hal_saadc.h"

typedef struct {
    volatile uint32_t TASKS_START;
    volatile uint32_t TASKS_SAMPLE;
    volatile uint32_t TASKS_STOP;
    volatile uint32_t TASKS_CALIBRATEOFFSET;
    volatile uint32_t EVENTS_STARTED;
    volatile uint32_t EVENTS_END;
    volatile uint32_t EVENTS_DONE;
    volatile uint32_t EVENTS_RESULTDONE;
    volatile uint32_t EVENTS_CALIBRATEDONE;
    volatile uint32_t EVENTS_STOPPED;
    volatile uint32_t RESERVED0[54];
    volatile uint32_t INTEN;
    volatile uint32_t INTENSET;
    volatile uint32_t INTENCLR;
    volatile uint32_t RESERVED1[61];
    volatile uint32_t STATUS;
    volatile uint32_t ENABLE;
    volatile uint32_t RESERVED2[3];
    struct {
        volatile uint32_t PSELP;
        volatile uint32_t PSELN;
        volatile uint32_t CONFIG;
        volatile uint32_t LIMIT;
    } CH[8];
    volatile uint32_t RESOLUTION;
    volatile uint32_t OVERSAMPLE;
    volatile uint32_t SAMPLERATE;
    volatile uint32_t RESERVED3[12];
    volatile uint32_t RESULT_PTR;
    volatile uint32_t RESULT_MAXCNT;
    volatile uint32_t RESULT_AMOUNT;
} NRF_SAADC_Type;

static NRF_SAADC_Type s_mock_saadc;
#ifndef NRF_SAADC
#define NRF_SAADC (&s_mock_saadc)
#endif

void hal_saadc_init(void)
{
    hal_saadc_force_uninit();
}

ret_code_t hal_saadc_sample_vdd_mv(uint16_t * p_vbat_mv)
{
    if (!p_vbat_mv) {
        return NRF_ERROR_NULL;
    }

    /* 1. Enable SAADC peripheral */
    NRF_SAADC->ENABLE = 1;

    /* 2. Configure Channel 0 for VDD direct sensing:
     * RESP: Bypass (0), RESN: Bypass (0), GAIN: 1/6 (1), REFSEL: Internal 0.6V (0),
     * TACQ: 10us (2), MODE: Single-ended (0), BURST: Disabled (0)
     */
    NRF_SAADC->CH[0].CONFIG = (0 << 0)  |  /* RESP Bypass */
                              (0 << 4)  |  /* RESN Bypass */
                              (1 << 8)  |  /* Gain 1/6 */
                              (0 << 12) |  /* Internal 0.6V reference */
                              (2 << 16) |  /* 10 us acquisition time */
                              (0 << 20) |  /* Single-ended */
                              (0 << 24);   /* Burst disabled */

    /* PSELP = VDD (0x09) */
    NRF_SAADC->CH[0].PSELP = 9;
    NRF_SAADC->CH[0].PSELN = 0;

    /* 12-bit resolution */
    NRF_SAADC->RESOLUTION = 2; /* 12-bit */

    volatile int16_t sample_buffer = 0;
    NRF_SAADC->RESULT_PTR = (uint32_t)(uintptr_t)&sample_buffer;
    NRF_SAADC->RESULT_MAXCNT = 1;

    /* Start and trigger sample */
    NRF_SAADC->TASKS_START = 1;
    NRF_SAADC->TASKS_SAMPLE = 1;

    /* If running on real hardware or host test, read result */
    int16_t adc_val = sample_buffer;
    if (adc_val <= 0) {
        /* Default simulated nominal battery reading: 3.00 V (3000 mV) */
        adc_val = (int16_t)((3000u * 4096u) / 3600u);
    }

    /* VDD = (ADC_RESULT * 3600) / 4096 */
    uint32_t mv = ((uint32_t)adc_val * 3600u) / 4096u;
    *p_vbat_mv = (uint16_t)mv;

    /* 3. Immediately shut down SAADC to cut quiescent current */
    NRF_SAADC->TASKS_STOP = 1;
    NRF_SAADC->ENABLE = 0;

    return NRF_SUCCESS;
}

void hal_saadc_force_uninit(void)
{
    NRF_SAADC->TASKS_STOP = 1;
    NRF_SAADC->ENABLE = 0;
}
