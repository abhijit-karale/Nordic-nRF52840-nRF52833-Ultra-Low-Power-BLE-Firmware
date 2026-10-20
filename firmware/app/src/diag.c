/**
 * @file diag.c
 * @brief Diagnostics, Fault Recording, and HardFault Capture Implementation
 */

#include "diag.h"
#include "protocol.h"
#include <string.h>

#if defined(__GNUC__)
    #define FAULT_SECTION __attribute__((section(".noinit.fault_record")))
#else
    #define FAULT_SECTION
#endif

static FAULT_SECTION hardfault_record_t s_fault_record;
static diag_counters_t s_diag_counters;

void diag_init(void)
{
    /* Check if previous HardFault record exists in uninitialized RAM */
    size_t data_len = offsetof(hardfault_record_t, crc16);
    if (s_fault_record.magic == FAULT_RECORD_MAGIC) {
        uint16_t crc = protocol_crc16((const uint8_t *)&s_fault_record, data_len);
        if (crc == s_fault_record.crc16) {
            s_diag_counters.hardfault_cnt++;
            s_diag_counters.last_fault_code = FAULT_HARDFAULT;
            s_diag_counters.last_fault_pc = s_fault_record.pc;
        }
        /* Invalidate record once accounted for */
        s_fault_record.magic = 0;
    }
}

void diag_record_fault(fault_code_t code, uint32_t pc)
{
    s_diag_counters.last_fault_code = code;
    s_diag_counters.last_fault_pc = pc;

    if (code == FAULT_WDT) {
        s_diag_counters.wdt_resets++;
    } else if (code == FAULT_RET_CRC) {
        s_diag_counters.ret_fails++;
    }
}

void diag_inc_bad_frames(void)
{
    s_diag_counters.bad_frames++;
}

void diag_inc_i2c_errors(void)
{
    s_diag_counters.i2c_errors++;
}

const diag_counters_t * diag_get_counters(void)
{
    return &s_diag_counters;
}

void diag_hardfault_handler_c(const uint32_t * p_stacked_args)
{
    if (!p_stacked_args) {
        return;
    }

    s_fault_record.magic = FAULT_RECORD_MAGIC;
    s_fault_record.r0    = p_stacked_args[0];
    s_fault_record.r1    = p_stacked_args[1];
    s_fault_record.r2    = p_stacked_args[2];
    s_fault_record.r3    = p_stacked_args[3];
    s_fault_record.r12   = p_stacked_args[4];
    s_fault_record.lr    = p_stacked_args[5];
    s_fault_record.pc    = p_stacked_args[6];
    s_fault_record.xpsr  = p_stacked_args[7];

    size_t data_len = offsetof(hardfault_record_t, crc16);
    s_fault_record.crc16 = protocol_crc16((const uint8_t *)&s_fault_record, data_len);

    /* Trigger software reset */
    NRF_POWER->RESETREAS = POWER_RESETREAS_LOCKUP_Msk;
#if defined(__GNUC__) && !defined(HOST_TEST)
    /* System reset register on Cortex-M */
    *(volatile uint32_t *)0xE000ED0C = 0x05FA0004;
#endif
    for (;;) { }
}

/* Default app_error_fault_handler required by APP_ERROR_CHECK */
void app_error_fault_handler(ret_code_t error_code, uint32_t line_num, uint32_t info)
{
    UNUSED_PARAMETER(error_code);
    UNUSED_PARAMETER(info);
    diag_record_fault(FAULT_ASSERT, line_num);
}
