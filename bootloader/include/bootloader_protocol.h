#ifndef BOOTLOADER_PROTOCOL_H
#define BOOTLOADER_PROTOCOL_H

#include "transport_interface.h"
#include "bootloader_errors.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_VERSION_MAJOR 1
#define PROTOCOL_VERSION_MINOR 0
#define PROTOCOL_MAX_COMMAND_LENGTH 32
#define PROTOCOL_MAX_RESPONSE_LENGTH 64
#define PROTOCOL_MAX_PAYLOAD_SIZE 256

typedef enum {
    PROTOCOL_STATE_IDLE = 0,
    PROTOCOL_STATE_SYNC,
    PROTOCOL_STATE_HANDSHAKE_COMPLETE,
    PROTOCOL_STATE_UPLOAD_START,
    PROTOCOL_STATE_DATA_TRANSFER,
    PROTOCOL_STATE_UPLOAD_COMPLETE,
    PROTOCOL_STATE_ERROR
} protocol_state_t;

typedef enum {
    CMD_SYNC = 0,
    CMD_VERSION,
    CMD_STATUS,
    CMD_UPLOAD_START,
    CMD_DATA,
    CMD_UPLOAD_COMPLETE,
    CMD_RESET,
    CMD_PING,
    CMD_INVALID = 0xFF
} protocol_command_t;

typedef enum {
    RESP_OK = 0,
    RESP_BOOTLOADER_READY,
    RESP_VERSION_INFO,
    RESP_STATUS_INFO,
    RESP_READY_FOR_DATA,
    RESP_CHUNK_OK,
    RESP_UPLOAD_SUCCESS,
    RESP_RESETTING,
    RESP_PONG,
    RESP_ERROR_INVALID_COMMAND,
    RESP_ERROR_INVALID_STATE,
    RESP_ERROR_INVALID_DATA,
    RESP_ERROR_TIMEOUT,
    RESP_ERROR_HARDWARE,
    RESP_ERROR = 0xFF
} protocol_response_t;

typedef struct {
    protocol_command_t command;
    uint8_t length;
    uint8_t data[PROTOCOL_MAX_PAYLOAD_SIZE];
    uint16_t crc16;
} protocol_message_t;

typedef struct {
    protocol_response_t response;
    uint8_t length;
    uint8_t data[PROTOCOL_MAX_PAYLOAD_SIZE];
    uint16_t crc16;
} protocol_response_message_t;

typedef struct {
    protocol_state_t state;
    transport_context_t* transport;
    
    uint8_t rx_buffer[PROTOCOL_MAX_COMMAND_LENGTH + 16];
    uint8_t tx_buffer[PROTOCOL_MAX_RESPONSE_LENGTH + 16];
    
    uint16_t rx_index;
    uint16_t tx_index;
    
    protocol_message_t current_message;
    protocol_response_message_t current_response;
    
    uint32_t last_activity_time;
    uint32_t handshake_time;
    
    uint32_t messages_received;
    uint32_t messages_sent;
    uint32_t errors_count;
    
    bool debug_enabled;
} protocol_context_t;

typedef struct {
    uint32_t total_size;
    uint32_t chunk_size;
    uint16_t total_chunks;
    uint16_t current_chunk;
    uint32_t bytes_received;
    uint16_t last_chunk_crc;
    bool upload_in_progress;
} upload_context_t;

void protocol_init(protocol_context_t* ctx, transport_context_t* transport);
void protocol_deinit(protocol_context_t* ctx);

protocol_response_t protocol_process_message(protocol_context_t* ctx);
protocol_response_t protocol_send_response(protocol_context_t* ctx, protocol_response_t response, const uint8_t* data, uint8_t length);

protocol_response_t protocol_handle_sync(protocol_context_t* ctx);
protocol_response_t protocol_handle_version(protocol_context_t* ctx);
protocol_response_t protocol_handle_status(protocol_context_t* ctx);
protocol_response_t protocol_handle_upload_start(protocol_context_t* ctx, const uint8_t* data, uint8_t length);
protocol_response_t protocol_handle_data(protocol_context_t* ctx, const uint8_t* data, uint8_t length);
protocol_response_t protocol_handle_upload_complete(protocol_context_t* ctx);
protocol_response_t protocol_handle_reset(protocol_context_t* ctx);
protocol_response_t protocol_handle_ping(protocol_context_t* ctx);

bool protocol_receive_message(protocol_context_t* ctx, uint32_t timeout_ms);
bool protocol_parse_message(protocol_context_t* ctx, protocol_message_t* message);

uint16_t protocol_calculate_crc16(const uint8_t* data, uint16_t length);
bool protocol_verify_crc16(const uint8_t* data, uint16_t length, uint16_t expected_crc);

const char* protocol_command_to_string(protocol_command_t command);
const char* protocol_response_to_string(protocol_response_t response);
const char* protocol_state_to_string(protocol_state_t state);

void protocol_set_debug(protocol_context_t* ctx, bool debug);
bool protocol_is_ready(protocol_context_t* ctx);
bool protocol_is_error_state(protocol_context_t* ctx);

extern upload_context_t g_upload_context;

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_PROTOCOL_H