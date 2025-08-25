/*
 * ComponentVM Bootloader Protocol
 * 
 * Binary protocol implementation with protobuf messages and CRC16 framing.
 * Built on Phase 4.5.1 bootloader foundation for blocking-first reliability.
 */

#ifndef BOOTLOADER_PROTOCOL_H
#define BOOTLOADER_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Generated nanopb headers
#include "bootloader.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Protocol constants
#define BOOTLOADER_PROTOCOL_VERSION "4.5.2"
#define BOOTLOADER_MAX_PAYLOAD_SIZE 1024
#define BOOTLOADER_FRAME_OVERHEAD   6    // START + LENGTH + CRC + END
#define BOOTLOADER_MAX_FRAME_SIZE   (BOOTLOADER_MAX_PAYLOAD_SIZE + BOOTLOADER_FRAME_OVERHEAD)

// Frame markers
#define BOOTLOADER_FRAME_START 0x7E
#define BOOTLOADER_FRAME_END   0x7F

// Flash programming constants (STM32G431CB specific)
#ifndef BOOTLOADER_TEST_PAGE_ADDR
#define BOOTLOADER_TEST_PAGE_ADDR 0x0801F800  // Page 63 - last bytecode page
#endif

#define BOOTLOADER_FLASH_PAGE_SIZE    2048    // STM32G431CB page size
#define BOOTLOADER_FLASH_WRITE_ALIGN  8       // 64-bit alignment required

// Protocol result codes
typedef enum {
    BOOTLOADER_PROTOCOL_SUCCESS = 0,
    BOOTLOADER_PROTOCOL_ERROR_FRAME_INVALID,
    BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH,
    BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE,
    BOOTLOADER_PROTOCOL_ERROR_TIMEOUT,
    BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_DECODE,
    BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_ENCODE,
    BOOTLOADER_PROTOCOL_ERROR_FLASH_OPERATION,
    BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID
} bootloader_protocol_result_t;

// Frame parser state machine
typedef enum {
    FRAME_STATE_IDLE = 0,
    FRAME_STATE_SYNC,
    FRAME_STATE_LENGTH_HIGH,
    FRAME_STATE_LENGTH_LOW, 
    FRAME_STATE_PAYLOAD,
    FRAME_STATE_CRC_HIGH,
    FRAME_STATE_CRC_LOW,
    FRAME_STATE_END,
    FRAME_STATE_COMPLETE
} frame_parse_state_t;

// Frame structure
typedef struct {
    uint8_t payload[BOOTLOADER_MAX_PAYLOAD_SIZE];
    uint16_t payload_length;
    uint16_t calculated_crc;
    uint16_t received_crc;
} bootloader_frame_t;

// Frame parser context  
typedef struct {
    frame_parse_state_t state;
    bootloader_frame_t frame;
    uint16_t bytes_received;
    uint32_t last_activity_time;  // For timeout detection
} frame_parser_t;

// Flash write context for 64-bit alignment
typedef struct {
    uint8_t staging_buffer[BOOTLOADER_FLASH_WRITE_ALIGN];
    uint32_t staging_offset;        // Current bytes in staging buffer
    uint32_t flash_write_address;   // Next aligned write address
    uint32_t actual_data_length;    // Original data length (without padding)
    bool page_erased;               // Track if page has been erased
} flash_write_context_t;

// Protocol context
typedef struct {
    frame_parser_t parser;
    flash_write_context_t flash_context;
    uint32_t sequence_counter;
    uint32_t timeout_ms;            // Default 500ms
} bootloader_protocol_context_t;

// CRC16 functions
uint16_t calculate_crc16_ccitt(const uint8_t* data, size_t length);
uint16_t calculate_frame_crc16(uint16_t length, const uint8_t* payload);

// Frame handling functions
void frame_parser_init(frame_parser_t* parser);
bootloader_protocol_result_t frame_parser_process_byte(frame_parser_t* parser, uint8_t byte);
bool frame_parser_is_complete(const frame_parser_t* parser);
void frame_parser_reset(frame_parser_t* parser);

// Frame encoding/decoding
bootloader_protocol_result_t frame_encode(const uint8_t* payload, uint16_t length, uint8_t* frame_buffer, size_t* frame_length);

// Protocol message handling
bootloader_protocol_result_t protocol_handle_request(const BootloaderRequest* request, BootloaderResponse* response);
bootloader_protocol_result_t protocol_encode_response(const BootloaderResponse* response, uint8_t* buffer, size_t* length);
bootloader_protocol_result_t protocol_decode_request(const uint8_t* buffer, size_t length, BootloaderRequest* request);

// Flash operations
bootloader_protocol_result_t flash_context_init(flash_write_context_t* ctx);
bootloader_protocol_result_t flash_erase_page(uint32_t page_address);
bootloader_protocol_result_t flash_stage_data(flash_write_context_t* ctx, const uint8_t* data, uint32_t length);
bootloader_protocol_result_t flash_flush_staging(flash_write_context_t* ctx);
bootloader_protocol_result_t flash_verify_data(uint32_t address, const uint8_t* expected_data, uint32_t length);

// Protocol state enumeration
typedef enum {
    PROTOCOL_STATE_IDLE = 0,
    PROTOCOL_STATE_HANDSHAKE_COMPLETE,
    PROTOCOL_STATE_READY_FOR_DATA,
    PROTOCOL_STATE_DATA_RECEIVED,
    PROTOCOL_STATE_PROGRAMMING_COMPLETE,
    PROTOCOL_STATE_ERROR
} protocol_state_t;

// Protocol context structure
typedef struct {
    flash_write_context_t flash_ctx;    // Flash operations context
    uint32_t sequence_counter;          // Current sequence ID
    protocol_state_t state;             // Protocol state machine
    uint32_t session_timeout_ms;        // Session timeout (30 seconds)
    uint32_t last_activity_time;        // Last valid message time
    
    // Single-transfer tracking
    bool data_received;                 // DataPacket received flag
    uint32_t expected_data_length;      // From prepare request
    uint32_t actual_data_length;        // From DataPacket
} protocol_context_t;

// Protocol context management
void protocol_init(void);
protocol_context_t* protocol_get_context(void);
void protocol_context_init(protocol_context_t* ctx);
bool protocol_is_session_timeout(const protocol_context_t* ctx);
void protocol_update_activity(protocol_context_t* ctx);
bootloader_protocol_result_t protocol_reset_session(protocol_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_PROTOCOL_H