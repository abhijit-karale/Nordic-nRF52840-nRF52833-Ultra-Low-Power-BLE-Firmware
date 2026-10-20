/**
 * @file sdk_config.h
 * @brief Nordic nRF5 SDK Configuration Header for ULP-BLE Node
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* Low-Power Logging: strictly disabled in production builds */
#if defined(DEBUG) || (defined(LOG) && (LOG == 1))
    #define NRF_LOG_ENABLED                     1
    #define NRF_LOG_BACKEND_RTT_ENABLED         1
    #define NRF_LOG_DEFAULT_LEVEL               3
#else
    #define NRF_LOG_ENABLED                     0
    #define NRF_LOG_BACKEND_RTT_ENABLED         0
    #define NRF_LOG_DEFAULT_LEVEL               0
#endif

/* Low Frequency Clock Configuration (LFXO 32.768 kHz External Crystal) */
#define NRF_SDH_CLOCK_LF_SRC                    1  /* NRF_CLOCK_LF_SRC_XTAL */
#define NRF_SDH_CLOCK_LF_RC_CTIV                0
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV           0
#define NRF_SDH_CLOCK_LF_ACCURACY               7  /* NRF_CLOCK_LF_ACCURACY_20_PPM */

/* SoftDevice BLE Configuration */
#define NRF_SDH_BLE_ENABLED                     1
#define NRF_SDH_BLE_GAP_DATA_LENGTH             27
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT       1
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT          0
#define NRF_SDH_BLE_TOTAL_LINK_COUNT            1
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE           23
#define NRF_BLE_GATT_ENABLED                    1
#define NRF_BLE_QWR_ENABLED                     1

/* Application Timer on RTC1 */
#define APP_TIMER_ENABLED                       1
#define APP_TIMER_CONFIG_RTC_FREQUENCY          1
#define APP_TIMER_V2                            1

/* Scheduler */
#define APP_SCHEDULER_ENABLED                   1

/* Power Management */
#define NRF_PWR_MGMT_ENABLED                    1
#define NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED   0
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S   0

/* Peripherals */
#define NRFX_TWIM_ENABLED                       1
#define NRFX_TWIM0_ENABLED                      1

#define NRFX_SAADC_ENABLED                      1

#define NRFX_WDT_ENABLED                        1

/* FreeRTOS variant flag */
#ifndef ULP_USE_FREERTOS
#define ULP_USE_FREERTOS                        0
#endif

#endif /* SDK_CONFIG_H */
