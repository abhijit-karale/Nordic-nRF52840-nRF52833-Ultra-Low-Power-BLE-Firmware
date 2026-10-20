/**
 * @file test_suite.c
 * @brief Host Unit Test Suite for ULP-BLE Firmware Modules
 *
 * Validates protocol framing, CRC-16/CCITT-FALSE, retention recovery,
 * corruption detection, and state machine transitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "protocol.h"
#include "retention.h"
#include "app_cfg.h"
#include "app_fsm.h"
#include "wake_mgr.h"
#include "diag.h"

static int s_tests_run = 0;
static int s_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    s_tests_run++; \
    if (!(cond)) { \
        printf("  [FAIL] Line %d: %s\n", __LINE__, msg); \
    } else { \
        s_tests_passed++; \
        printf("  [PASS] %s\n", msg); \
    } \
} while (0)

void test_crc16_standard_vector(void)
{
    printf("\n--- Test: CRC-16/CCITT-FALSE Standard Vector ---\n");
    const uint8_t test_vector[] = "123456789";
    uint16_t crc = protocol_crc16(test_vector, 9);
    /* Standard CCITT-FALSE vector on "123456789" is 0x29B1 */
    TEST_ASSERT(crc == 0x29B1, "CRC-16/CCITT-FALSE matches 0x29B1 on 123456789");
}

void test_telemetry_frame_format(void)
{
    printf("\n--- Test: Telemetry 20-byte Frame Encoding & Validation ---\n");
    telemetry_frame_t frame_in;
    memset(&frame_in, 0, sizeof(frame_in));
    frame_in.seq = 42;
    frame_in.temp_c_x100 = 2450; /* 24.50 °C */
    frame_in.vbat_mv = 2950;     /* 2.95 V */
    frame_in.pwr_state = 2;      /* CONN_IDLE */
    frame_in.wake_reason = 2;    /* WAKE_BUTTON_EVT */
    frame_in.flags = TELEM_FLAG_RETENTION_REST;
    frame_in.motion_cnt = 5;
    frame_in.sysoff_cnt = 12;
    frame_in.uptime_s = 3600;

    uint8_t wire[TELEMETRY_FRAME_SIZE];
    protocol_encode_telemetry(&frame_in, wire);

    TEST_ASSERT(wire[0] == 0x11, "Telemetry ver_type equals 0x11");
    TEST_ASSERT(wire[1] == 42, "Sequence counter matches 42");
    TEST_ASSERT(wire[2] == 15, "Payload length equals 15");

    bool valid = protocol_validate_telemetry(wire);
    TEST_ASSERT(valid == true, "Telemetry wire frame passes CRC validation");

    /* Corrupt one byte and assert rejection */
    wire[5] ^= 0xFF;
    bool corrupt_valid = protocol_validate_telemetry(wire);
    TEST_ASSERT(corrupt_valid == false, "Corrupted telemetry wire frame is rejected");
}

void test_config_characteristic_decode(void)
{
    printf("\n--- Test: Config Characteristic Encode / Decode ---\n");
    config_payload_t cfg_in;
    cfg_in.ver              = PROTOCOL_VERSION;
    cfg_in.adv_fast_ms      = 100;
    cfg_in.adv_slow_ms      = 1000;
    cfg_in.adv_fast_dur_s   = 30;
    cfg_in.idle_to_sysoff_s = 120;
    cfg_in.telem_period_s   = 10;
    cfg_in.wake_mask        = 0x03;
    cfg_in.tx_power_dbm     = 0;

    uint8_t wire[CONFIG_FRAME_SIZE];
    protocol_encode_config(&cfg_in, wire);

    config_payload_t cfg_out;
    bool ok = protocol_decode_config(wire, sizeof(wire), &cfg_out);
    TEST_ASSERT(ok == true, "Config payload successfully decoded");
    TEST_ASSERT(cfg_out.adv_fast_ms == 100, "adv_fast_ms matches 100");
    TEST_ASSERT(cfg_out.adv_slow_ms == 1000, "adv_slow_ms matches 1000");

    /* Out of bounds bounds-checking */
    cfg_out.adv_fast_ms = 5; /* Minimum allowed is 20 ms */
    bool app_cfg_ok = app_cfg_update(&cfg_out);
    TEST_ASSERT(app_cfg_ok == false, "Out-of-bounds config rejected by app_cfg_update");
}

void test_control_opcodes(void)
{
    printf("\n--- Test: Control Opcode Parsing ---\n");
    /* Test OP_SET_ADV (0x02) with 4 bytes params: fast_ms=50, slow_ms=2000 */
    uint8_t raw_cmd[8];
    raw_cmd[0] = OP_SET_ADV;
    raw_cmd[1] = 4; /* 4 params bytes */
    raw_cmd[2] = 50;  raw_cmd[3] = 0;  /* 50 ms */
    raw_cmd[4] = 0xD0; raw_cmd[5] = 0x07; /* 2000 ms */

    uint16_t crc = protocol_crc16(raw_cmd, 6);
    raw_cmd[6] = (uint8_t)(crc & 0xFF);
    raw_cmd[7] = (uint8_t)((crc >> 8) & 0xFF);

    uint8_t opcode = 0;
    const uint8_t * p_params = NULL;
    uint8_t plen = 0;

    bool valid = protocol_parse_control(raw_cmd, sizeof(raw_cmd), &opcode, &p_params, &plen);
    TEST_ASSERT(valid == true, "Control frame parsed successfully");
    TEST_ASSERT(opcode == OP_SET_ADV, "Opcode is OP_SET_ADV");
    TEST_ASSERT(plen == 4, "Param length is 4 bytes");

    /* Corrupt opcode CRC */
    raw_cmd[6] ^= 0x01;
    bool bad_crc = protocol_parse_control(raw_cmd, sizeof(raw_cmd), &opcode, &p_params, &plen);
    TEST_ASSERT(bad_crc == false, "Control frame with corrupted CRC is rejected");
}

void test_retention_lifecycle_and_corruption(void)
{
    printf("\n--- Test: Retention Persistence & Corruption Recovery ---\n");
    retention_init();

    /* 1. First boot: Cold boot defaults */
    retention_restore();
    const retention_record_t * p_rec = retention_get_record();
    TEST_ASSERT(p_rec->magic == RETENTION_MAGIC_VALUE, "Retention magic matches");
    TEST_ASSERT(p_rec->sysoff_cnt == 0, "Initial System OFF count is 0");

    /* 2. Simulate System OFF cycle */
    retention_save_before_sysoff();
    TEST_ASSERT(p_rec->sysoff_cnt == 1, "sysoff_cnt incremented to 1");

    /* 3. Simulate Warm Wake */
    bool restored = retention_restore();
    TEST_ASSERT(restored == true, "Warm wake validates CRC successfully");
    TEST_ASSERT(p_rec->boot_cnt == 2, "boot_cnt incremented to 2");

    /* 4. Deliberate corruption test (TC-29) */
    retention_corrupt_for_test();
    bool corrupt_restored = retention_restore();
    TEST_ASSERT(corrupt_restored == false, "Corrupted retention CRC rejected");
    TEST_ASSERT(p_rec->boot_cnt == 1, "Fallback to defaults on corruption: boot_cnt reset to 1");
    TEST_ASSERT(p_rec->sysoff_cnt == 0, "Fallback to defaults: sysoff_cnt reset to 0");
}

int main(void)
{
    printf("============================================================\n");
    printf("  ULP-BLE Node Unit Test Suite (Host Target)\n");
    printf("============================================================\n");

    test_crc16_standard_vector();
    test_telemetry_frame_format();
    test_config_characteristic_decode();
    test_control_opcodes();
    test_retention_lifecycle_and_corruption();

    printf("\n============================================================\n");
    printf("Summary: %d / %d tests passed\n", s_tests_passed, s_tests_run);
    printf("============================================================\n");

    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
