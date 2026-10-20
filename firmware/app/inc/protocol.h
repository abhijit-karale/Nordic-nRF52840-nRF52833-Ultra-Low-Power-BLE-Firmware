/**
 * @file protocol.h
 * @brief Communication Frame Format, CRC-16, and Encoding/Decoding
 *
 * Implements 20-byte Telemetry frame, Config characteristic encoding/decoding,
 * Control opcode parsing, and CRC-16/CCITT-FALSE validation.
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_VERSION            1
#define MSG_TYPE_TELEMETRY          1

#define TELEMETRY_FRAME_SIZE        20
#define CONFIG_FRAME_SIZE           15
#define CONTROL_HEADER_SIZE         2
#define CONTROL_CRC_SIZE            2

/* Flags bits in telemetry frame */
#define TELEM_FLAG_LOW_BAT          (1u << 0)
#define TELEM_FLAG_SENSOR_FAULT     (1u << 1)
#define TELEM_FLAG_RETENTION_REST   (1u << 2)
#define TELEM_FLAG_MOTION_SINCE_LST (1u << 3)

/* Control Opcodes */
#define OP_ENTER_SYSOFF             0x01
#define OP_SET_ADV                  0x02
#define OP_SET_TELEM                0x03
#define OP_GET_STATUS               0x04
#define OP_CLEAR_COUNTERS           0x05
#define OP_SOFT_RESET               0x06

#define RESET_MAGIC_KEY             0xA55A

#pragma pack(push, 1)

/**
 * @brief Telemetry Notification Frame (Exactly 20 bytes, fits standard ATT MTU)
 */
typedef struct {
    uint8_t  ver_type;      /* [7:4] version=1, [3:0] type=1 -> 0x11 */
    uint8_t  seq;           /* Rolling sequence counter */
    uint8_t  len;           /* Payload length = 15 */
    int16_t  temp_c_x100;   /* Temperature in centi-degrees Celsius (int16 LE) */
    uint16_t vbat_mv;       /* Battery voltage in millivolts (uint16 LE) */
    uint8_t  pwr_state;     /* 0:ADV_FAST, 1:ADV_SLOW, 2:CONN_IDLE, 3:CONN_ACTIVE */
    uint8_t  wake_reason;   /* wake_reason_t from boot */
    uint8_t  flags;         /* Status flags */
    uint16_t motion_cnt;    /* Retained motion event counter */
    uint16_t sysoff_cnt;    /* Retained System OFF entry counter */
    uint32_t uptime_s;      /* Total seconds since cold boot */
    uint16_t crc16;         /* CRC-16/CCITT-FALSE over bytes 0..17 */
} telemetry_frame_t;

/**
 * @brief Config Characteristic Payload (15 bytes)
 */
typedef struct {
    uint8_t  ver;           /* 1 */
    uint16_t adv_fast_ms;   /* 20 - 1000 ms */
    uint16_t adv_slow_ms;   /* 100 - 10240 ms */
    uint16_t adv_fast_dur_s;/* 1 - 600 s */
    uint16_t idle_to_sysoff_s; /* 10 - 65535 s (0 = never) */
    uint16_t telem_period_s;/* 1 - 3600 s */
    uint8_t  wake_mask;     /* b0: button, b1: motion, b2: ext RTC */
    int8_t   tx_power_dbm;  /* Radio TX power in dBm */
    uint16_t crc16;         /* CRC-16 over bytes 0..12 */
} config_payload_t;

/**
 * @brief Status Characteristic Payload (24 bytes)
 */
typedef struct {
    uint8_t  ver;
    uint8_t  pwr_state;
    uint8_t  wake_reason;
    uint8_t  last_fault_code;
    uint16_t boot_cnt;
    uint16_t bad_frame_cnt;
    uint16_t i2c_error_cnt;
    uint16_t ret_fail_cnt;
    uint32_t last_fault_pc;
    uint32_t uptime_s;
    uint16_t vbat_mv;
    uint16_t crc16;
} status_payload_t;

#pragma pack(pop)

/**
 * @brief Compute CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF, no reflect, xorout 0x0000).
 */
uint16_t protocol_crc16(const uint8_t * p_data, size_t length);

/**
 * @brief Encode a telemetry struct into a 20-byte wire frame and compute its CRC-16.
 */
void protocol_encode_telemetry(const telemetry_frame_t * p_in, uint8_t * p_out_20b);

/**
 * @brief Validate a received 20-byte telemetry frame.
 */
bool protocol_validate_telemetry(const uint8_t * p_frame_20b);

/**
 * @brief Decode and validate a Config characteristic write.
 */
bool protocol_decode_config(const uint8_t * p_buf, size_t len, config_payload_t * p_cfg_out);

/**
 * @brief Encode Config struct with updated CRC-16.
 */
void protocol_encode_config(const config_payload_t * p_cfg, uint8_t * p_buf_out);

/**
 * @brief Parse and validate a Control opcode frame [opcode:1][len:1][params:len][crc16:2].
 *
 * @param p_buf Input frame buffer.
 * @param len Total frame length.
 * @param[out] p_opcode Extracted opcode.
 * @param[out] pp_params Pointer to start of parameters.
 * @param[out] p_param_len Length of parameters.
 * @return true if CRC and length match, false otherwise.
 */
bool protocol_parse_control(const uint8_t * p_buf, size_t len,
                            uint8_t * p_opcode, const uint8_t ** pp_params, uint8_t * p_param_len);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H_ */
