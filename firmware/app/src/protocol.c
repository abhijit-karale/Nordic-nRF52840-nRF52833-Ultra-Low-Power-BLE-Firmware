/**
 * @file protocol.c
 * @brief Communication Frame Format, CRC-16, and Encoding/Decoding Implementation
 */

#include "protocol.h"
#include <string.h>

uint16_t protocol_crc16(const uint8_t * p_data, size_t length)
{
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)((uint16_t)p_data[i] << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}

void protocol_encode_telemetry(const telemetry_frame_t * p_in, uint8_t * p_out_20b)
{
    if (!p_in || !p_out_20b) {
        return;
    }

    /* Copy first 18 bytes of frame */
    memcpy(p_out_20b, p_in, 18);

    /* Enforce fixed version/type and payload length */
    p_out_20b[0] = (uint8_t)((PROTOCOL_VERSION << 4) | (MSG_TYPE_TELEMETRY & 0x0F));
    p_out_20b[2] = 15; /* 15 payload bytes */

    /* Compute CRC-16 over bytes 0..17 */
    uint16_t crc = protocol_crc16(p_out_20b, 18);

    /* Little-endian CRC serialization */
    p_out_20b[18] = (uint8_t)(crc & 0xFF);
    p_out_20b[19] = (uint8_t)((crc >> 8) & 0xFF);
}

bool protocol_validate_telemetry(const uint8_t * p_frame_20b)
{
    if (!p_frame_20b) {
        return false;
    }

    uint8_t ver = (p_frame_20b[0] >> 4) & 0x0F;
    uint8_t type = p_frame_20b[0] & 0x0F;
    uint8_t len = p_frame_20b[2];

    if (ver != PROTOCOL_VERSION || type != MSG_TYPE_TELEMETRY || len != 15) {
        return false;
    }

    uint16_t expected_crc = (uint16_t)(p_frame_20b[18] | ((uint16_t)p_frame_20b[19] << 8));
    uint16_t calculated_crc = protocol_crc16(p_frame_20b, 18);

    return (expected_crc == calculated_crc);
}

void protocol_encode_config(const config_payload_t * p_cfg, uint8_t * p_buf_out)
{
    if (!p_cfg || !p_buf_out) {
        return;
    }

    memcpy(p_buf_out, p_cfg, CONFIG_FRAME_SIZE - 2);
    p_buf_out[0] = PROTOCOL_VERSION;

    uint16_t crc = protocol_crc16(p_buf_out, CONFIG_FRAME_SIZE - 2);
    p_buf_out[CONFIG_FRAME_SIZE - 2] = (uint8_t)(crc & 0xFF);
    p_buf_out[CONFIG_FRAME_SIZE - 1] = (uint8_t)((crc >> 8) & 0xFF);
}

bool protocol_decode_config(const uint8_t * p_buf, size_t len, config_payload_t * p_cfg_out)
{
    if (!p_buf || !p_cfg_out || len != CONFIG_FRAME_SIZE) {
        return false;
    }

    uint16_t expected_crc = (uint16_t)(p_buf[CONFIG_FRAME_SIZE - 2] |
                                      ((uint16_t)p_buf[CONFIG_FRAME_SIZE - 1] << 8));
    uint16_t calculated_crc = protocol_crc16(p_buf, CONFIG_FRAME_SIZE - 2);

    if (expected_crc != calculated_crc) {
        return false;
    }

    memcpy(p_cfg_out, p_buf, sizeof(config_payload_t));
    if (p_cfg_out->ver != PROTOCOL_VERSION) {
        return false;
    }

    return true;
}

bool protocol_parse_control(const uint8_t * p_buf, size_t len,
                            uint8_t * p_opcode, const uint8_t ** pp_params, uint8_t * p_param_len)
{
    if (!p_buf || !p_opcode || !pp_params || !p_param_len) {
        return false;
    }

    /* Minimum frame is [opcode:1][plen:1][crc16:2] = 4 bytes */
    if (len < (CONTROL_HEADER_SIZE + CONTROL_CRC_SIZE)) {
        return false;
    }

    uint8_t plen = p_buf[1];
    if (len != (size_t)(CONTROL_HEADER_SIZE + plen + CONTROL_CRC_SIZE)) {
        return false;
    }

    size_t payload_len = CONTROL_HEADER_SIZE + plen;
    uint16_t expected_crc = (uint16_t)(p_buf[payload_len] | ((uint16_t)p_buf[payload_len + 1] << 8));
    uint16_t calculated_crc = protocol_crc16(p_buf, payload_len);

    if (expected_crc != calculated_crc) {
        return false;
    }

    *p_opcode = p_buf[0];
    *p_param_len = plen;
    *pp_params = (plen > 0) ? &p_buf[CONTROL_HEADER_SIZE] : NULL;

    return true;
}
