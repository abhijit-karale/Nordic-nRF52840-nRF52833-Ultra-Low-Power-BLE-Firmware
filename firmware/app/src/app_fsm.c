/**
 * @file app_fsm.c
 * @brief Application Power and Connection State Machine Implementation
 */

#include "app_fsm.h"
#include "app_cfg.h"
#include "power_mgr.h"
#include "wake_mgr.h"
#include "telemetry.h"
#include "retention.h"
#include "diag.h"
#include "drv_vbat.h"
#include "ble_ulp_svc.h"

static app_state_t s_current_state = S_BOOT;
static uint16_t    s_conn_handle = BLE_CONN_HANDLE_INVALID;
static uint8_t     s_override_wake_mask = 0;

APP_TIMER_DEF(m_adv_timer);
APP_TIMER_DEF(m_telem_timer);
APP_TIMER_DEF(m_wdt_feed_timer);

static void adv_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    if (s_current_state == S_ADV_FAST) {
        app_fsm_dispatch(FSM_EVT_ADV_FAST_TIMEOUT);
    } else if (s_current_state == S_ADV_SLOW) {
        app_fsm_dispatch(FSM_EVT_ADV_SLOW_TIMEOUT);
    }
}

static void telem_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    app_fsm_dispatch(FSM_EVT_TELEM_TIMER);
}

static void wdt_feed_timer_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    /* Feed watchdog health monitor tick */
    hal_wdt_feed_health_bit(HEALTH_BIT_TIMER);
    hal_wdt_process_health_check();
}

ret_code_t app_fsm_init(void)
{
    s_current_state = S_BOOT;
    s_conn_handle = BLE_CONN_HANDLE_INVALID;

    ret_code_t err;
    err = app_timer_create(&m_adv_timer, APP_TIMER_MODE_SINGLE_SHOT, adv_timer_timeout_handler);
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) return err;

    err = app_timer_create(&m_telem_timer, APP_TIMER_MODE_REPEATED, telem_timer_timeout_handler);
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) return err;

    err = app_timer_create(&m_wdt_feed_timer, APP_TIMER_MODE_REPEATED, wdt_feed_timer_timeout_handler);
    if (err != NRF_SUCCESS && err != NRF_ERROR_NOT_SUPPORTED) return err;

    /* Start periodic watchdog health check (every 4 seconds) */
    app_timer_start(m_wdt_feed_timer, APP_TIMER_TICKS(4000), NULL);

    return NRF_SUCCESS;
}

void app_fsm_start(void)
{
    app_fsm_dispatch(FSM_EVT_INIT_OK);
}

app_state_t app_fsm_get_state(void)
{
    return s_current_state;
}

void app_fsm_set_conn_handle(uint16_t conn_handle)
{
    s_conn_handle = conn_handle;
}

void app_fsm_dispatch(fsm_event_t event)
{
    const config_payload_t * p_cfg = app_cfg_get();

    switch (s_current_state) {
        case S_BOOT:
            if (event == FSM_EVT_INIT_OK) {
                /* Start Fast Advertising */
                sd_ble_gap_tx_power_set(0, 0, p_cfg->tx_power_dbm);
                sd_ble_gap_adv_start(0, 1);

                app_timer_start(m_adv_timer, APP_TIMER_TICKS((uint32_t)p_cfg->adv_fast_dur_s * 1000u), NULL);
                s_current_state = S_ADV_FAST;
            }
            break;

        case S_ADV_FAST:
            if (event == FSM_EVT_CONNECTED) {
                app_timer_stop(m_adv_timer);
                app_timer_start(m_telem_timer, APP_TIMER_TICKS((uint32_t)p_cfg->telem_period_s * 1000u), NULL);
                s_current_state = S_CONN_IDLE;
            } else if (event == FSM_EVT_ADV_FAST_TIMEOUT) {
                /* Transition to Slow Advertising */
                sd_ble_gap_adv_stop(0);
                sd_ble_gap_adv_start(0, 1);

                if (p_cfg->idle_to_sysoff_s > 0) {
                    app_timer_start(m_adv_timer, APP_TIMER_TICKS((uint32_t)p_cfg->idle_to_sysoff_s * 1000u), NULL);
                }
                s_current_state = S_ADV_SLOW;
            }
            break;

        case S_ADV_SLOW:
            if (event == FSM_EVT_CONNECTED) {
                app_timer_stop(m_adv_timer);
                app_timer_start(m_telem_timer, APP_TIMER_TICKS((uint32_t)p_cfg->telem_period_s * 1000u), NULL);
                s_current_state = S_CONN_IDLE;
            } else if (event == FSM_EVT_ADV_SLOW_TIMEOUT) {
                sd_ble_gap_adv_stop(0);
                s_current_state = S_PREP_SYSOFF;
                power_mgr_enter_system_off(p_cfg->wake_mask);
            }
            break;

        case S_CONN_IDLE:
            if (event == FSM_EVT_TELEM_TIMER) {
                s_current_state = S_CONN_ACTIVE;
                telemetry_send_notification(s_conn_handle);

                /* Check battery condition after reading */
                if (drv_vbat_is_critical()) {
                    app_fsm_dispatch(FSM_EVT_LOW_BAT_CRITICAL);
                    return;
                }
                s_current_state = S_CONN_IDLE;
            } else if (event == FSM_EVT_DISCONNECTED) {
                app_timer_stop(m_telem_timer);
                s_conn_handle = BLE_CONN_HANDLE_INVALID;

                sd_ble_gap_adv_start(0, 1);
                app_timer_start(m_adv_timer, APP_TIMER_TICKS((uint32_t)p_cfg->adv_fast_dur_s * 1000u), NULL);
                s_current_state = S_ADV_FAST;
            } else if (event == FSM_EVT_ENTER_SYSOFF_CMD) {
                app_timer_stop(m_telem_timer);
                sd_ble_gap_disconnect(s_conn_handle, 0x13 /* Remote User Terminated */);
                s_current_state = S_PREP_SYSOFF;
                uint8_t mask = s_override_wake_mask ? s_override_wake_mask : p_cfg->wake_mask;
                power_mgr_enter_system_off(mask);
            }
            break;

        case S_CONN_ACTIVE:
            if (event == FSM_EVT_TELEM_DONE) {
                s_current_state = S_CONN_IDLE;
            }
            break;

        case S_PREP_SYSOFF:
            /* Awaiting hardware sleep completion */
            break;

        case S_FAULT:
            /* Logged, pending system reset */
            break;

        default:
            break;
    }

    if (event == FSM_EVT_LOW_BAT_CRITICAL) {
        if (s_conn_handle != BLE_CONN_HANDLE_INVALID) {
            sd_ble_gap_disconnect(s_conn_handle, 0x15 /* Terminated due to low resources */);
        }
        sd_ble_gap_adv_stop(0);
        app_timer_stop(m_adv_timer);
        app_timer_stop(m_telem_timer);

        s_current_state = S_PREP_SYSOFF;
        /* Low battery safety: wake strictly on button only */
        power_mgr_enter_system_off(WAKE_MASK_BUTTON);
    }
}

void app_fsm_on_control_command(uint8_t opcode, const uint8_t * p_params, uint8_t len)
{
    switch (opcode) {
        case OP_ENTER_SYSOFF: {
            s_override_wake_mask = (len >= 1 && p_params) ? p_params[0] : 0;
            app_fsm_dispatch(FSM_EVT_ENTER_SYSOFF_CMD);
            break;
        }

        case OP_SET_ADV: {
            if (len >= 4 && p_params) {
                uint16_t fast_ms = (uint16_t)(p_params[0] | ((uint16_t)p_params[1] << 8));
                uint16_t slow_ms = (uint16_t)(p_params[2] | ((uint16_t)p_params[3] << 8));
                app_cfg_set_adv(fast_ms, slow_ms);
            }
            break;
        }

        case OP_SET_TELEM: {
            if (len >= 2 && p_params) {
                uint16_t period_s = (uint16_t)(p_params[0] | ((uint16_t)p_params[1] << 8));
                app_cfg_set_telem_period(period_s);
            }
            break;
        }

        case OP_GET_STATUS: {
            status_payload_t status;
            memset(&status, 0, sizeof(status));
            status.ver             = PROTOCOL_VERSION;
            status.pwr_state       = (uint8_t)s_current_state;
            status.wake_reason     = (uint8_t)wake_mgr_get_last_reason();

            const diag_counters_t * p_diag = diag_get_counters();
            const retention_record_t * p_ret = retention_get_record();

            if (p_diag) {
                status.last_fault_code = (uint8_t)p_diag->last_fault_code;
                status.bad_frame_cnt   = p_diag->bad_frames;
                status.i2c_error_cnt   = p_diag->i2c_errors;
                status.ret_fail_cnt    = p_diag->ret_fails;
                status.last_fault_pc   = p_diag->last_fault_pc;
            }
            if (p_ret) {
                status.boot_cnt = p_ret->boot_cnt;
                status.uptime_s = p_ret->uptime_s;
            }

            drv_vbat_sample_mv(&status.vbat_mv);

            size_t data_len = offsetof(status_payload_t, crc16);
            status.crc16 = protocol_crc16((const uint8_t *)&status, data_len);
            ble_ulp_svc_update_status(&status);
            break;
        }

        case OP_CLEAR_COUNTERS: {
            retention_clear_counters();
            break;
        }

        case OP_SOFT_RESET: {
            if (len >= 2 && p_params) {
                uint16_t magic = (uint16_t)(p_params[0] | ((uint16_t)p_params[1] << 8));
                if (magic == RESET_MAGIC_KEY) {
                    NRF_POWER->RESETREAS = POWER_RESETREAS_SREQ_Msk;
#if defined(__GNUC__) && !defined(HOST_TEST)
                    *(volatile uint32_t *)0xE000ED0C = 0x05FA0004;
#endif
                }
            }
            break;
        }

        default:
            diag_inc_bad_frames();
            break;
    }
}
