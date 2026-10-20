/**
 * @file app_cfg.c
 * @brief Application Runtime Configuration and Defaults Implementation
 */

#include "app_cfg.h"
#include <string.h>

static config_payload_t s_active_cfg;

void app_cfg_init(void)
{
    s_active_cfg.ver              = PROTOCOL_VERSION;
    s_active_cfg.adv_fast_ms      = DEFAULT_ADV_FAST_MS;
    s_active_cfg.adv_slow_ms      = DEFAULT_ADV_SLOW_MS;
    s_active_cfg.adv_fast_dur_s   = DEFAULT_ADV_FAST_DUR_S;
    s_active_cfg.idle_to_sysoff_s = DEFAULT_IDLE_TO_SYSOFF_S;
    s_active_cfg.telem_period_s   = DEFAULT_TELEM_PERIOD_S;
    s_active_cfg.wake_mask        = DEFAULT_WAKE_MASK;
    s_active_cfg.tx_power_dbm     = DEFAULT_TX_POWER_DBM;

    uint8_t temp_buf[CONFIG_FRAME_SIZE];
    protocol_encode_config(&s_active_cfg, temp_buf);
    s_active_cfg.crc16 = (uint16_t)(temp_buf[CONFIG_FRAME_SIZE - 2] |
                                   ((uint16_t)temp_buf[CONFIG_FRAME_SIZE - 1] << 8));
}

const config_payload_t * app_cfg_get(void)
{
    return &s_active_cfg;
}

bool app_cfg_update(const config_payload_t * p_new_cfg)
{
    if (!p_new_cfg) {
        return false;
    }

    /* Validate bounds */
    if (p_new_cfg->adv_fast_ms < 20 || p_new_cfg->adv_fast_ms > 1000) {
        return false;
    }
    if (p_new_cfg->adv_slow_ms < 100 || p_new_cfg->adv_slow_ms > 10240) {
        return false;
    }
    if (p_new_cfg->adv_fast_dur_s < 1 || p_new_cfg->adv_fast_dur_s > 600) {
        return false;
    }
    if (p_new_cfg->telem_period_s < 1 || p_new_cfg->telem_period_s > 3600) {
        return false;
    }

    s_active_cfg = *p_new_cfg;
    return true;
}

bool app_cfg_set_adv(uint16_t fast_ms, uint16_t slow_ms)
{
    if (fast_ms < 20 || fast_ms > 1000 || slow_ms < 100 || slow_ms > 10240) {
        return false;
    }

    s_active_cfg.adv_fast_ms = fast_ms;
    s_active_cfg.adv_slow_ms = slow_ms;
    return true;
}

bool app_cfg_set_telem_period(uint16_t period_s)
{
    if (period_s < 1 || period_s > 3600) {
        return false;
    }

    s_active_cfg.telem_period_s = period_s;
    return true;
}
