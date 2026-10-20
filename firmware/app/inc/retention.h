/**
 * @file retention.h
 * @brief State Retention Across System OFF Deep Sleep
 *
 * Implements CRC-16 validated state retention within a retained .noinit RAM section,
 * GPREGRET hardware register flag persistence, and fallback to defaults on corruption.
 */

#ifndef RETENTION_H_
#define RETENTION_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RETENTION_MAGIC_VALUE       0x55AA1234
#define RETENTION_STRUCT_VERSION    1

#define GPREGRET_FLAG_WARM_BOOT     (1u << 0)
#define GPREGRET_FLAG_USER_SHUTDOWN (1u << 1)
#define GPREGRET_FLAG_LOW_BAT_SLEEP (1u << 2)

#pragma pack(push, 1)

/**
 * @brief Retained state structure residing in .noinit.retention RAM
 */
typedef struct {
    uint32_t magic;         /* RETENTION_MAGIC_VALUE */
    uint16_t version;       /* RETENTION_STRUCT_VERSION */
    uint16_t sysoff_cnt;    /* Count of System OFF entries */
    uint16_t motion_cnt;    /* Total motion events detected */
    uint16_t boot_cnt;      /* Total boot cycles */
    uint32_t uptime_s;      /* Accumulated seconds of operation */
    uint16_t last_vbat_mv;  /* Last measured battery voltage */
    uint16_t crc16;         /* CRC-16/CCITT-FALSE over preceding bytes */
} retention_record_t;

#pragma pack(pop)

/**
 * @brief Initialize retention subsystem.
 */
void retention_init(void);

/**
 * @brief Validate and restore retained state across System OFF.
 *
 * @return true if record validated successfully, false if corrupt or cold boot.
 */
bool retention_restore(void);

/**
 * @brief Prepare and persist retention data before entering System OFF.
 * Increments sysoff_cnt, serializes GPREGRET, updates uptime, and calculates CRC-16.
 */
void retention_save_before_sysoff(void);

/**
 * @brief Increment retained motion counter.
 */
void retention_inc_motion_cnt(void);

/**
 * @brief Update accumulated uptime.
 *
 * @param additional_seconds Elapsed seconds to add.
 */
void retention_add_uptime(uint32_t additional_seconds);

/**
 * @brief Clear all retained counters (via Control characteristic opcode 0x05).
 */
void retention_clear_counters(void);

/**
 * @brief Deliberately corrupt the retention record CRC (for TC-29 verification).
 */
void retention_corrupt_for_test(void);

/**
 * @brief Access current retention record.
 */
const retention_record_t * retention_get_record(void);

#ifdef __cplusplus
}
#endif

#endif /* RETENTION_H_ */
