/**
 * @file sdk_common.h
 * @brief Common types, macros and includes for Nordic SDK compatibility
 */

#ifndef SDK_COMMON_H__
#define SDK_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_power.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "ble_gap_gatts.h"

#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(X)  ((void)(X))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(X) ((void)(X))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif /* SDK_COMMON_H__ */
