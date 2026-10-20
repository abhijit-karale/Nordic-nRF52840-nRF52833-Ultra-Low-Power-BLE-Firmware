/**
 * @file app_fsm.h
 * @brief Application Power and Connection State Machine
 *
 * Implements the 8-state power lifecycle:
 * S_BOOT -> S_ADV_FAST -> S_ADV_SLOW -> S_PREP_SYSOFF -> S_SYSOFF
 * Connected branch: S_CONN_IDLE <-> S_CONN_ACTIVE
 * Exception handling: S_FAULT
 */

#ifndef APP_FSM_H_
#define APP_FSM_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    S_BOOT = 0,
    S_ADV_FAST,
    S_ADV_SLOW,
    S_CONN_IDLE,
    S_CONN_ACTIVE,
    S_PREP_SYSOFF,
    S_SYSOFF,
    S_FAULT
} app_state_t;

typedef enum {
    FSM_EVT_INIT_OK = 0,
    FSM_EVT_ADV_FAST_TIMEOUT,
    FSM_EVT_ADV_SLOW_TIMEOUT,
    FSM_EVT_CONNECTED,
    FSM_EVT_DISCONNECTED,
    FSM_EVT_TELEM_TIMER,
    FSM_EVT_TELEM_DONE,
    FSM_EVT_ENTER_SYSOFF_CMD,
    FSM_EVT_LOW_BAT_CRITICAL,
    FSM_EVT_FAULT
} fsm_event_t;

/**
 * @brief Initialize state machine and timers.
 */
ret_code_t app_fsm_init(void);

/**
 * @brief Start the state machine following boot initialization.
 */
void app_fsm_start(void);

/**
 * @brief Dispatch an event to the state machine.
 *
 * @param event FSM event.
 */
void app_fsm_dispatch(fsm_event_t event);

/**
 * @brief Get current state.
 */
app_state_t app_fsm_get_state(void);

/**
 * @brief Set current BLE connection handle.
 */
void app_fsm_set_conn_handle(uint16_t conn_handle);

/**
 * @brief Handle Control characteristic opcode commands.
 */
void app_fsm_on_control_command(uint8_t opcode, const uint8_t * p_params, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* APP_FSM_H_ */
