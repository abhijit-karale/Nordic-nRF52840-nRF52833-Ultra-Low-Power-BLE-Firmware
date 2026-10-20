/**
 * @file diag.h
 * @brief Diagnostics, Fault Recording, and HardFault Capture
 *
 * Implements fault code tracking, HardFault stacked CPU context capture into
 * .noinit memory, and operational error counters.
 */

#ifndef DIAG_H_
#define DIAG_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FAULT_NONE = 0,
    FAULT_I2C_TIMEOUT,
    FAULT_SENSOR_ID,
    FAULT_RET_CRC,
    FAULT_ADV_START,
    FAULT_ASSERT,
    FAULT_WDT,
    FAULT_HARDFAULT
} fault_code_t;

#pragma pack(push, 1)

/**
 * @brief CPU context captured at HardFault exception
 */
typedef struct {
    uint32_t magic;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
    uint16_t crc16;
} hardfault_record_t;

/**
 * @brief Diagnostic error and event counters
 */
typedef struct {
    uint16_t bad_frames;
    uint16_t i2c_errors;
    uint16_t ret_fails;
    uint16_t wdt_resets;
    uint16_t hardfault_cnt;
    fault_code_t last_fault_code;
    uint32_t last_fault_pc;
} diag_counters_t;

#pragma pack(pop)

#define FAULT_RECORD_MAGIC 0xDEADBEEF

/**
 * @brief Initialize diagnostic counters and check for pending HardFault record.
 */
void diag_init(void);

/**
 * @brief Record a system fault.
 *
 * @param code Fault code.
 * @param pc Program counter where fault originated.
 */
void diag_record_fault(fault_code_t code, uint32_t pc);

/**
 * @brief Increment bad frame counter (corrupt length/CRC).
 */
void diag_inc_bad_frames(void);

/**
 * @brief Increment I2C bus error counter.
 */
void diag_inc_i2c_errors(void);

/**
 * @brief Get diagnostic counters structure.
 */
const diag_counters_t * diag_get_counters(void);

/**
 * @brief Save HardFault stacked frame (called from assembly or C exception handler).
 */
void diag_hardfault_handler_c(const uint32_t * p_stacked_args);

#ifdef __cplusplus
}
#endif

#endif /* DIAG_H_ */
