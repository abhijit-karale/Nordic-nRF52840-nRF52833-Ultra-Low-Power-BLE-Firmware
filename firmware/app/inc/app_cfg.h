/**
 * @file app_cfg.h
 * @brief Application Runtime Configuration and Defaults
 */

#ifndef APP_CFG_H_
#define APP_CFG_H_

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_ADV_FAST_MS         100     /* 100 ms fast advertising */
#define DEFAULT_ADV_SLOW_MS         1000    /* 1000 ms slow advertising */
#define DEFAULT_ADV_FAST_DUR_S      30      /* 30 seconds fast window */
#define DEFAULT_IDLE_TO_SYSOFF_S    120     /* 120 seconds idle before System OFF */
#define DEFAULT_TELEM_PERIOD_S      10      /* 10 seconds telemetry notify */
#define DEFAULT_WAKE_MASK           0x03    /* Button (b0) + Motion (b1) */
#define DEFAULT_TX_POWER_DBM        0       /* 0 dBm default TX power */

#define DEFAULT_CONN_MIN_INTERVAL_MS 200
#define DEFAULT_CONN_MAX_INTERVAL_MS 1000
#define DEFAULT_CONN_PREF_INTERVAL_MS 500
#define DEFAULT_CONN_SLAVE_LATENCY   4
#define DEFAULT_CONN_SUP_TIMEOUT_MS  6000

/**
 * @brief Initialize application configuration with factory defaults.
 */
void app_cfg_init(void);

/**
 * @brief Retrieve pointer to current active configuration.
 */
const config_payload_t * app_cfg_get(void);

/**
 * @brief Update configuration from incoming Config characteristic write.
 * Validates parameter bounds before accepting.
 *
 * @param p_new_cfg New configuration data.
 * @return true if valid and applied, false if any parameter out of bounds.
 */
bool app_cfg_update(const config_payload_t * p_new_cfg);

/**
 * @brief Update only advertising parameters (via OP_SET_ADV opcode).
 */
bool app_cfg_set_adv(uint16_t fast_ms, uint16_t slow_ms);

/**
 * @brief Update only telemetry period (via OP_SET_TELEM opcode).
 */
bool app_cfg_set_telem_period(uint16_t period_s);

#ifdef __cplusplus
}
#endif

#endif /* APP_CFG_H_ */
