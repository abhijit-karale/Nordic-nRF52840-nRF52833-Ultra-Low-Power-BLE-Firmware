/**
 * @file app_timer.h
 * @brief Application timer API wrapper
 */

#ifndef APP_TIMER_H__
#define APP_TIMER_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"

typedef struct {
    uint32_t placeholder;
} app_timer_t;

typedef app_timer_t* app_timer_id_t;
typedef void (*app_timer_timeout_handler_t)(void * p_context);

#define APP_TIMER_DEF(timer_id) \
    static app_timer_t timer_id##_data; \
    static const app_timer_id_t timer_id = &timer_id##_data

typedef enum {
    APP_TIMER_MODE_SINGLE_SHOT,
    APP_TIMER_MODE_REPEATED
} app_timer_mode_t;

#define APP_TIMER_TICKS(MS) ((uint32_t)(((uint64_t)(MS) * 32768) / 1000))

ret_code_t app_timer_init(void);
ret_code_t app_timer_create(app_timer_id_t const * p_timer_id,
                            app_timer_mode_t mode,
                            app_timer_timeout_handler_t timeout_handler);
ret_code_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ticks, void * p_context);
ret_code_t app_timer_stop(app_timer_id_t timer_id);

#endif /* APP_TIMER_H__ */
