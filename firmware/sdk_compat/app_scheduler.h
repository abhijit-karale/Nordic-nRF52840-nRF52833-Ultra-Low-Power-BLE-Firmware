/**
 * @file app_scheduler.h
 * @brief Application event scheduler definitions
 */

#ifndef APP_SCHEDULER_H__
#define APP_SCHEDULER_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"

typedef void (*app_sched_event_handler_t)(void * p_event_data, uint16_t event_size);

ret_code_t app_sched_init(uint16_t max_event_size, uint16_t queue_size, void * p_evt_buffer);
ret_code_t app_sched_event_put(void * p_event_data, uint16_t event_size, app_sched_event_handler_t handler);
void app_sched_execute(void);

#define APP_SCHED_INIT(EVENT_SIZE, QUEUE_SIZE) \
    app_sched_init((EVENT_SIZE), (QUEUE_SIZE), 0)

#endif /* APP_SCHEDULER_H__ */
