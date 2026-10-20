/**
 * @file app_main.c
 * @brief Application Entry Point and Main Run Loop Implementation
 */

#include "app_main.h"
#include "board_config.h"
#include "hal_gpio_lp.h"
#include "hal_twi.h"
#include "hal_saadc.h"
#include "hal_wdt.h"
#include "drv_tmp117.h"
#include "drv_lis2dw12.h"
#include "drv_vbat.h"
#include "wake_mgr.h"
#include "retention.h"
#include "diag.h"
#include "app_cfg.h"
#include "telemetry.h"
#include "power_mgr.h"
#include "ble_ulp_svc.h"
#include "app_fsm.h"

#define SCHED_MAX_EVENT_DATA_SIZE   sizeof(uint32_t)
#define SCHED_QUEUE_SIZE            16

#define DEVICE_NAME_PREFIX          "ULP-BLE"

static void ble_evt_dispatch_handler(const ble_evt_t * p_ble_evt)
{
    /* Feed watchdog health monitor that BLE stack is active */
    hal_wdt_feed_health_bit(HEALTH_BIT_BLE);

    switch (p_ble_evt->evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            app_fsm_set_conn_handle(p_ble_evt->evt.gap_evt.conn_handle);
            app_fsm_dispatch(FSM_EVT_CONNECTED);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            app_fsm_set_conn_handle(BLE_CONN_HANDLE_INVALID);
            app_fsm_dispatch(FSM_EVT_DISCONNECTED);
            break;

        default:
            break;
    }

    /* Forward event to GATT service */
    ble_ulp_svc_on_ble_evt(p_ble_evt);
}

void board_early_init(void)
{
    /* Configure all unused pins into lowest power state (input disconnect, no pull) */
    hal_gpio_lp_init_all_unused();
    hal_twi_init();
    hal_saadc_init();
    diag_init();
}

static void log_init_if_enabled(void)
{
#if defined(DEBUG) || (defined(LOG) && (LOG == 1))
    /* Logging initialized only in development builds */
#else
    /* Strictly compiled out in production builds to preserve uA sleep current */
#endif
}

static void timers_init(void)
{
    ret_code_t err = app_timer_init();
    APP_ERROR_CHECK(err);

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

ret_code_t ble_stack_init(void)
{
    /* In actual SDK builds, nrf_sdh_enable_request() is called here */
    return NRF_SUCCESS;
}

ret_code_t gap_gatt_services_init(void)
{
    /* Initialize application configuration defaults */
    app_cfg_init();

    /* Initialize telemetry orchestrator */
    telemetry_init();

    /* Initialize Custom ULP GATT Service with Control opcode callback */
    ret_code_t err = ble_ulp_svc_init(app_fsm_on_control_command);
    APP_ERROR_CHECK(err);

    return NRF_SUCCESS;
}

int main(void)
{
    /* 1. Early board and GPIO low-power initialization */
    board_early_init();

    /* 2. Capture reset reason before SoftDevice is enabled */
    wake_mgr_capture_reset_reason();

    /* 3. Logging (compiled out in production) */
    log_init_if_enabled();

    /* 4. Initialize RTC1 app_timer and scheduler */
    timers_init();

    /* 5. Initialize power management and enable DC/DC converter */
    power_mgr_init();

    /* 6. SoftDevice stack enable */
    ble_stack_init();

    /* 7. GAP, GATT, and Custom ULP service registration */
    gap_gatt_services_init();

    /* 8. Hardware sensor driver bring-up */
    drv_tmp117_init();
    drv_lis2dw12_init();
    drv_vbat_init();

    /* 9. Retention validation: restore counters or set defaults on corruption */
    retention_restore();

    /* 10. Hardware Watchdog initialization with 10s timeout */
    hal_wdt_init(10000);

    /* 11. Start Power & Connection state machine */
    app_fsm_init();
    app_fsm_start();

    /* 12. Main event loop */
    for (;;) {
        /* Record main loop health bit for watchdog */
        hal_wdt_feed_health_bit(HEALTH_BIT_MAIN);

        /* Process pending events posted to scheduler */
        app_sched_execute();

        /* Enter lowest power System ON sleep until next RTC/GPIO/BLE interrupt */
        power_mgr_run_sleep();
    }

    return 0;
}
