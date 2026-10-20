/**
 * @file retention.c
 * @brief State Retention Across System OFF Deep Sleep Implementation
 */

#include "retention.h"
#include "protocol.h"
#include <string.h>

#if defined(__GNUC__)
    #define RETENTION_SECTION __attribute__((section(".noinit.retention")))
#else
    #define RETENTION_SECTION
#endif

/* Retained record placed in .noinit section */
static RETENTION_SECTION retention_record_t s_retained_record;

static bool s_retention_valid = false;
static uint16_t s_retention_fail_cnt = 0;

static void load_defaults(void)
{
    memset(&s_retained_record, 0, sizeof(retention_record_t));
    s_retained_record.magic = RETENTION_MAGIC_VALUE;
    s_retained_record.version = RETENTION_STRUCT_VERSION;
    s_retained_record.boot_cnt = 1;
    s_retained_record.sysoff_cnt = 0;
    s_retained_record.motion_cnt = 0;
    s_retained_record.uptime_s = 0;
    s_retained_record.last_vbat_mv = 3000;

    size_t data_len = offsetof(retention_record_t, crc16);
    s_retained_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);
    s_retention_valid = false;
}

void retention_init(void)
{
    /* Nothing required prior to restore */
}

bool retention_restore(void)
{
    size_t data_len = offsetof(retention_record_t, crc16);

    /* Check magic number and version */
    if (s_retained_record.magic != RETENTION_MAGIC_VALUE ||
        s_retained_record.version != RETENTION_STRUCT_VERSION)
    {
        s_retention_fail_cnt++;
        load_defaults();
        return false;
    }

    /* Validate CRC-16 */
    uint16_t computed_crc = protocol_crc16((const uint8_t *)&s_retained_record, data_len);
    if (computed_crc != s_retained_record.crc16) {
        s_retention_fail_cnt++;
        load_defaults();
        return false;
    }

    /* Valid restore: increment boot count and mark restored */
    s_retained_record.boot_cnt++;
    s_retention_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);

    s_retention_valid = true;
    return true;
}

void retention_save_before_sysoff(void)
{
    s_retained_record.magic = RETENTION_MAGIC_VALUE;
    s_retained_record.version = RETENTION_STRUCT_VERSION;
    s_retained_record.sysoff_cnt++;

    size_t data_len = offsetof(retention_record_t, crc16);
    s_retained_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);

    /* Set GPREGRET hardware register flag */
    NRF_POWER->GPREGRET |= GPREGRET_FLAG_WARM_BOOT;
}

void retention_inc_motion_cnt(void)
{
    s_retained_record.motion_cnt++;
    size_t data_len = offsetof(retention_record_t, crc16);
    s_retained_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);
}

void retention_add_uptime(uint32_t additional_seconds)
{
    s_retained_record.uptime_s += additional_seconds;
    size_t data_len = offsetof(retention_record_t, crc16);
    s_retained_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);
}

void retention_clear_counters(void)
{
    s_retained_record.sysoff_cnt = 0;
    s_retained_record.motion_cnt = 0;
    s_retained_record.uptime_s = 0;

    size_t data_len = offsetof(retention_record_t, crc16);
    s_retained_record.crc16 = protocol_crc16((const uint8_t *)&s_retained_record, data_len);
}

void retention_corrupt_for_test(void)
{
    /* Corrupt the CRC field deliberately */
    s_retained_record.crc16 ^= 0xFFFF;
}

const retention_record_t * retention_get_record(void)
{
    return &s_retained_record;
}
