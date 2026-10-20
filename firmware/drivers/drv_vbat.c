/**
 * @file drv_vbat.c
 * @brief Battery Voltage Monitoring and Health Policy Driver Implementation
 */

#include "drv_vbat.h"
#include "hal_saadc.h"

#define VBAT_FILTER_SAMPLES 4

static uint16_t s_sample_history[VBAT_FILTER_SAMPLES];
static uint8_t  s_sample_count = 0;
static uint8_t  s_sample_idx = 0;
static uint16_t s_current_vbat_mv = 3000;
static vbat_status_level_t s_current_level = VBAT_LEVEL_OK;

ret_code_t drv_vbat_init(void)
{
    hal_saadc_init();

    s_sample_count = 0;
    s_sample_idx = 0;

    /* Prime moving average filter with initial samples */
    for (int i = 0; i < VBAT_FILTER_SAMPLES; i++) {
        uint16_t raw_mv = 3000;
        hal_saadc_sample_vdd_mv(&raw_mv);
        s_sample_history[i] = raw_mv;
    }
    s_sample_count = VBAT_FILTER_SAMPLES;
    s_current_vbat_mv = s_sample_history[0];
    s_current_level = VBAT_LEVEL_OK;

    return NRF_SUCCESS;
}

ret_code_t drv_vbat_sample_mv(uint16_t * p_vbat_mv)
{
    if (!p_vbat_mv) {
        return NRF_ERROR_NULL;
    }

    uint16_t raw_sample = 0;
    ret_code_t err = hal_saadc_sample_vdd_mv(&raw_sample);
    if (err != NRF_SUCCESS) {
        *p_vbat_mv = s_current_vbat_mv;
        return err;
    }

    /* Insert into circular buffer */
    s_sample_history[s_sample_idx] = raw_sample;
    s_sample_idx = (s_sample_idx + 1) % VBAT_FILTER_SAMPLES;
    if (s_sample_count < VBAT_FILTER_SAMPLES) {
        s_sample_count++;
    }

    /* Compute moving average */
    uint32_t sum = 0;
    for (uint8_t i = 0; i < s_sample_count; i++) {
        sum += s_sample_history[i];
    }
    s_current_vbat_mv = (uint16_t)(sum / s_sample_count);

    /* Evaluate battery status levels */
    if (s_current_vbat_mv < VBAT_CRITICAL_THRESHOLD_MV) {
        s_current_level = VBAT_LEVEL_CRITICAL;
    } else if (s_current_vbat_mv < VBAT_WARNING_THRESHOLD_MV) {
        s_current_level = VBAT_LEVEL_WARNING;
    } else {
        s_current_level = VBAT_LEVEL_OK;
    }

    *p_vbat_mv = s_current_vbat_mv;
    return NRF_SUCCESS;
}

vbat_status_level_t drv_vbat_get_level(void)
{
    return s_current_level;
}

bool drv_vbat_is_critical(void)
{
    return (s_current_level == VBAT_LEVEL_CRITICAL);
}
