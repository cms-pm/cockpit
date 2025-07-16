#include "bootloader_protocol.h"
#include <string.h>

upload_context_t g_upload_context = {0};

static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint16_t protocol_calculate_crc16(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0x0000;
    
    for (uint16_t i = 0; i < length; i++) {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ data[i]) & 0xFF];
    }
    
    return crc;
}

bool protocol_verify_crc16(const uint8_t* data, uint16_t length, uint16_t expected_crc) {
    uint16_t calculated_crc = protocol_calculate_crc16(data, length);
    return calculated_crc == expected_crc;
}

void protocol_init(protocol_context_t* ctx, transport_context_t* transport) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(protocol_context_t));
    ctx->transport = transport;
    ctx->state = PROTOCOL_STATE_IDLE;
    ctx->last_activity_time = get_system_tick_safe();
}

void protocol_deinit(protocol_context_t* ctx) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(protocol_context_t));
}

bool protocol_receive_message(protocol_context_t* ctx, uint32_t timeout_ms) {
    if (!ctx || !ctx->transport) return false;
    
    uint16_t bytes_available = 0;
    transport_status_t status = transport_available(ctx->transport, &bytes_available);
    
    if (status != TRANSPORT_OK || bytes_available == 0) {
        return false;
    }
    
    uint16_t bytes_to_read = bytes_available;
    if (bytes_to_read > (sizeof(ctx->rx_buffer) - ctx->rx_index)) {
        bytes_to_read = sizeof(ctx->rx_buffer) - ctx->rx_index;
    }
    
    uint16_t actual_bytes = 0;
    status = transport_receive(ctx->transport, &ctx->rx_buffer[ctx->rx_index], 
                              bytes_to_read, &actual_bytes, timeout_ms);
    
    if (status == TRANSPORT_OK && actual_bytes > 0) {
        ctx->rx_index += actual_bytes;
        ctx->last_activity_time = get_system_tick_safe();
        return true;
    }
    
    return false;
}

bool protocol_parse_message(protocol_context_t* ctx, protocol_message_t* message) {
    if (!ctx || !message || ctx->rx_index < 4) {
        return false;
    }
    
    message->command = (protocol_command_t)ctx->rx_buffer[0];
    message->length = ctx->rx_buffer[1];
    
    if (message->length > PROTOCOL_MAX_PAYLOAD_SIZE) {
        ctx->rx_index = 0;
        return false;
    }
    
    uint16_t total_length = 2 + message->length + 2;
    if (ctx->rx_index < total_length) {
        return false;
    }
    
    memcpy(message->data, &ctx->rx_buffer[2], message->length);
    message->crc16 = (ctx->rx_buffer[2 + message->length] << 8) | 
                     ctx->rx_buffer[2 + message->length + 1];
    
    if (!protocol_verify_crc16(ctx->rx_buffer, 2 + message->length, message->crc16)) {
        ctx->rx_index = 0;
        ctx->errors_count++;
        return false;
    }
    
    memmove(ctx->rx_buffer, &ctx->rx_buffer[total_length], ctx->rx_index - total_length);
    ctx->rx_index -= total_length;
    
    return true;
}

protocol_response_t protocol_send_response(protocol_context_t* ctx, protocol_response_t response, const uint8_t* data, uint8_t length) {
    if (!ctx || !ctx->transport) return RESP_ERROR_HARDWARE;
    
    if (length > PROTOCOL_MAX_PAYLOAD_SIZE) {
        return RESP_ERROR_INVALID_DATA;
    }
    
    ctx->tx_buffer[0] = (uint8_t)response;
    ctx->tx_buffer[1] = length;
    
    if (data && length > 0) {
        memcpy(&ctx->tx_buffer[2], data, length);
    }
    
    uint16_t crc = protocol_calculate_crc16(ctx->tx_buffer, 2 + length);
    ctx->tx_buffer[2 + length] = (crc >> 8) & 0xFF;
    ctx->tx_buffer[2 + length + 1] = crc & 0xFF;
    
    uint16_t total_length = 2 + length + 2;
    
    transport_status_t status = transport_send(ctx->transport, ctx->tx_buffer, total_length, 1000);
    
    if (status == TRANSPORT_OK) {
        ctx->messages_sent++;
        return response;
    } else {
        ctx->errors_count++;
        return RESP_ERROR_HARDWARE;
    }
}

protocol_response_t protocol_handle_sync(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    ctx->state = PROTOCOL_STATE_SYNC;
    
    const char* response_data = "BOOTLOADER_READY v1.0";
    return protocol_send_response(ctx, RESP_BOOTLOADER_READY, 
                                 (const uint8_t*)response_data, strlen(response_data));
}

protocol_response_t protocol_handle_version(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    uint8_t version_data[4];
    version_data[0] = PROTOCOL_VERSION_MAJOR;
    version_data[1] = PROTOCOL_VERSION_MINOR;
    version_data[2] = 0;
    version_data[3] = 1;
    
    return protocol_send_response(ctx, RESP_VERSION_INFO, version_data, 4);
}

protocol_response_t protocol_handle_status(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    uint8_t status_data[8];
    status_data[0] = (uint8_t)ctx->state;
    status_data[1] = 0;
    status_data[2] = (ctx->messages_received >> 8) & 0xFF;
    status_data[3] = ctx->messages_received & 0xFF;
    status_data[4] = (ctx->messages_sent >> 8) & 0xFF;
    status_data[5] = ctx->messages_sent & 0xFF;
    status_data[6] = (ctx->errors_count >> 8) & 0xFF;
    status_data[7] = ctx->errors_count & 0xFF;
    
    return protocol_send_response(ctx, RESP_STATUS_INFO, status_data, 8);
}

protocol_response_t protocol_handle_upload_start(protocol_context_t* ctx, const uint8_t* data, uint8_t length) {
    if (!ctx || !data || length < 4) return RESP_ERROR_INVALID_DATA;
    
    g_upload_context.total_size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    g_upload_context.chunk_size = 256;
    g_upload_context.total_chunks = (g_upload_context.total_size + g_upload_context.chunk_size - 1) / g_upload_context.chunk_size;
    g_upload_context.current_chunk = 0;
    g_upload_context.bytes_received = 0;
    g_upload_context.upload_in_progress = true;
    
    ctx->state = PROTOCOL_STATE_UPLOAD_START;
    
    return protocol_send_response(ctx, RESP_READY_FOR_DATA, NULL, 0);
}

protocol_response_t protocol_handle_data(protocol_context_t* ctx, const uint8_t* data, uint8_t length) {
    if (!ctx || !data) return RESP_ERROR_INVALID_DATA;
    
    if (!g_upload_context.upload_in_progress) {
        return RESP_ERROR_INVALID_STATE;
    }
    
    if (length < 3) {
        return RESP_ERROR_INVALID_DATA;
    }
    
    uint16_t chunk_crc = (data[length - 2] << 8) | data[length - 1];
    uint8_t chunk_data_length = length - 2;
    
    if (!protocol_verify_crc16(data, chunk_data_length, chunk_crc)) {
        return RESP_ERROR_INVALID_DATA;
    }
    
    g_upload_context.bytes_received += chunk_data_length;
    g_upload_context.current_chunk++;
    g_upload_context.last_chunk_crc = chunk_crc;
    
    ctx->state = PROTOCOL_STATE_DATA_TRANSFER;
    
    return protocol_send_response(ctx, RESP_CHUNK_OK, NULL, 0);
}

protocol_response_t protocol_handle_upload_complete(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    if (!g_upload_context.upload_in_progress) {
        return RESP_ERROR_INVALID_STATE;
    }
    
    g_upload_context.upload_in_progress = false;
    ctx->state = PROTOCOL_STATE_UPLOAD_COMPLETE;
    
    return protocol_send_response(ctx, RESP_UPLOAD_SUCCESS, NULL, 0);
}

protocol_response_t protocol_handle_reset(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    protocol_response_t response = protocol_send_response(ctx, RESP_RESETTING, NULL, 0);
    
    return response;
}

protocol_response_t protocol_handle_ping(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    return protocol_send_response(ctx, RESP_PONG, NULL, 0);
}

protocol_response_t protocol_process_message(protocol_context_t* ctx) {
    if (!ctx) return RESP_ERROR_HARDWARE;
    
    if (!protocol_receive_message(ctx, 100)) {
        return RESP_OK;
    }
    
    protocol_message_t message;
    if (!protocol_parse_message(ctx, &message)) {
        return RESP_ERROR_INVALID_DATA;
    }
    
    ctx->messages_received++;
    
    switch (message.command) {
        case CMD_SYNC:
            return protocol_handle_sync(ctx);
        case CMD_VERSION:
            return protocol_handle_version(ctx);
        case CMD_STATUS:
            return protocol_handle_status(ctx);
        case CMD_UPLOAD_START:
            return protocol_handle_upload_start(ctx, message.data, message.length);
        case CMD_DATA:
            return protocol_handle_data(ctx, message.data, message.length);
        case CMD_UPLOAD_COMPLETE:
            return protocol_handle_upload_complete(ctx);
        case CMD_RESET:
            return protocol_handle_reset(ctx);
        case CMD_PING:
            return protocol_handle_ping(ctx);
        default:
            ctx->errors_count++;
            return protocol_send_response(ctx, RESP_ERROR_INVALID_COMMAND, NULL, 0);
    }
}

const char* protocol_command_to_string(protocol_command_t command) {
    switch (command) {
        case CMD_SYNC: return "SYNC";
        case CMD_VERSION: return "VERSION";
        case CMD_STATUS: return "STATUS";
        case CMD_UPLOAD_START: return "UPLOAD_START";
        case CMD_DATA: return "DATA";
        case CMD_UPLOAD_COMPLETE: return "UPLOAD_COMPLETE";
        case CMD_RESET: return "RESET";
        case CMD_PING: return "PING";
        default: return "INVALID";
    }
}

const char* protocol_response_to_string(protocol_response_t response) {
    switch (response) {
        case RESP_OK: return "OK";
        case RESP_BOOTLOADER_READY: return "BOOTLOADER_READY";
        case RESP_VERSION_INFO: return "VERSION_INFO";
        case RESP_STATUS_INFO: return "STATUS_INFO";
        case RESP_READY_FOR_DATA: return "READY_FOR_DATA";
        case RESP_CHUNK_OK: return "CHUNK_OK";
        case RESP_UPLOAD_SUCCESS: return "UPLOAD_SUCCESS";
        case RESP_RESETTING: return "RESETTING";
        case RESP_PONG: return "PONG";
        case RESP_ERROR_INVALID_COMMAND: return "ERROR_INVALID_COMMAND";
        case RESP_ERROR_INVALID_STATE: return "ERROR_INVALID_STATE";
        case RESP_ERROR_INVALID_DATA: return "ERROR_INVALID_DATA";
        case RESP_ERROR_TIMEOUT: return "ERROR_TIMEOUT";
        case RESP_ERROR_HARDWARE: return "ERROR_HARDWARE";
        default: return "ERROR";
    }
}

const char* protocol_state_to_string(protocol_state_t state) {
    switch (state) {
        case PROTOCOL_STATE_IDLE: return "IDLE";
        case PROTOCOL_STATE_SYNC: return "SYNC";
        case PROTOCOL_STATE_HANDSHAKE_COMPLETE: return "HANDSHAKE_COMPLETE";
        case PROTOCOL_STATE_UPLOAD_START: return "UPLOAD_START";
        case PROTOCOL_STATE_DATA_TRANSFER: return "DATA_TRANSFER";
        case PROTOCOL_STATE_UPLOAD_COMPLETE: return "UPLOAD_COMPLETE";
        case PROTOCOL_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void protocol_set_debug(protocol_context_t* ctx, bool debug) {
    if (!ctx) return;
    
    ctx->debug_enabled = debug;
}

bool protocol_is_ready(protocol_context_t* ctx) {
    if (!ctx) return false;
    
    return ctx->state >= PROTOCOL_STATE_SYNC;
}

bool protocol_is_error_state(protocol_context_t* ctx) {
    if (!ctx) return true;
    
    return ctx->state == PROTOCOL_STATE_ERROR;
}