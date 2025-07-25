/*
 * CockpitVM Bootloader - Complete Protocol Engine Integration
 * 
 * Full Oracle-compatible protocol processing with:
 * - Complete nanopb protobuf message handling
 * - Binary framing with CRC16-CCITT validation
 * - Complete state machine transitions
 * - Flash programming with atomic operations
 * - Error recovery and timeout management
 */

#include "bootloader_protocol.h"
#include "context_internal.h"
#include "vm_bootloader.h"
#include "host_interface/host_interface.h"

#include <string.h>
#include <stdio.h>
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include "utilities/bootloader.pb.h"

// Global protocol context and frame parser
static protocol_context_t g_protocol_context;
static frame_parser_t g_frame_parser;
static bool g_protocol_initialized = false;

// Oracle-clean diagnostic output control
static bool g_diagnostics_enabled = false;

// Message buffers for protobuf encoding/decoding
static uint8_t g_request_buffer[BOOTLOADER_MAX_FRAME_SIZE];
static uint8_t g_response_buffer[BOOTLOADER_MAX_FRAME_SIZE];
static BootloaderRequest g_current_request;
static BootloaderResponse g_current_response;

// Forward declarations
static void vm_bootloader_protocol_init_internal(void);
static bool vm_bootloader_protocol_process_uart_data(vm_bootloader_context_internal_t* ctx);
static bootloader_protocol_result_t vm_bootloader_protocol_handle_frame(vm_bootloader_context_internal_t* ctx, const bootloader_frame_t* frame);
static bootloader_protocol_result_t vm_bootloader_protocol_send_response(const BootloaderResponse* response);
static void vm_bootloader_protocol_update_session_state(vm_bootloader_context_internal_t* ctx);

// Oracle-clean diagnostic output functions - DISABLED during frame transmission
void diagnostic_char(char c) {
    // Disable diagnostics completely during Oracle protocol to prevent frame corruption
    // The diagnostics were interfering with frame structure
    (void)c; // Suppress unused parameter warning
}

void vm_bootloader_enable_diagnostics_after_handshake(void) {
    // Disable diagnostic output completely to prevent Oracle frame corruption
    g_diagnostics_enabled = false;
}

// === PROTOCOL ENGINE API ===

void vm_bootloader_protocol_engine_init(void)
{
    if (!g_protocol_initialized) {
        vm_bootloader_protocol_init_internal();
        g_protocol_initialized = true;
    }
}

protocol_context_t* vm_bootloader_protocol_get_context(void)
{
    if (!g_protocol_initialized) {
        vm_bootloader_protocol_engine_init();
    }
    return &g_protocol_context;
}

bool vm_bootloader_protocol_process_frame(vm_bootloader_context_internal_t* ctx)
{
    if (!ctx || !g_protocol_initialized) {
        return false;
    }
    
    return vm_bootloader_protocol_process_uart_data(ctx);
}

void vm_bootloader_protocol_update_activity(void)
{
    if (g_protocol_initialized) {
        g_protocol_context.last_activity_time = get_tick_ms();
    }
}

void vm_bootloader_protocol_reset_session(void)
{
    if (g_protocol_initialized) {
        // Reset protocol state to IDLE
        g_protocol_context.state = PROTOCOL_STATE_IDLE;
        g_protocol_context.sequence_counter = 0;
        g_protocol_context.data_received = false;
        g_protocol_context.expected_data_length = 0;
        g_protocol_context.actual_data_length = 0;
        
        // Reset frame parser
        frame_parser_reset(&g_frame_parser);
        
        // Update activity timestamp
        g_protocol_context.last_activity_time = get_tick_ms();
    }
}

bool vm_bootloader_protocol_is_session_timeout(void)
{
    if (!g_protocol_initialized) {
        return false;
    }
    
    uint32_t current_time = get_tick_ms();
    uint32_t elapsed;
    
    // Overflow-safe timeout calculation
    if (current_time >= g_protocol_context.last_activity_time) {
        elapsed = current_time - g_protocol_context.last_activity_time;
    } else {
        elapsed = (UINT32_MAX - g_protocol_context.last_activity_time) + current_time + 1;
    }
    
    return elapsed >= g_protocol_context.session_timeout_ms;
}

vm_bootloader_state_t vm_bootloader_protocol_get_state(void)
{
    if (!g_protocol_initialized) {
        return VM_BOOTLOADER_STATE_INIT;
    }
    
    // Map protocol states to VM bootloader states
    switch (g_protocol_context.state) {
        case PROTOCOL_STATE_IDLE:
            return VM_BOOTLOADER_STATE_IDLE;
        case PROTOCOL_STATE_HANDSHAKE_COMPLETE:
            return VM_BOOTLOADER_STATE_HANDSHAKE;
        case PROTOCOL_STATE_READY_FOR_DATA:
            return VM_BOOTLOADER_STATE_READY;
        case PROTOCOL_STATE_DATA_RECEIVED:
            return VM_BOOTLOADER_STATE_RECEIVE_DATA;
        case PROTOCOL_STATE_PROGRAMMING_COMPLETE:
            return VM_BOOTLOADER_STATE_COMPLETE;
        case PROTOCOL_STATE_ERROR:
            return VM_BOOTLOADER_STATE_ERROR_COMMUNICATION;
        default:
            return VM_BOOTLOADER_STATE_ERROR_COMMUNICATION;
    }
}

// === INTERNAL PROTOCOL IMPLEMENTATION ===

static void vm_bootloader_protocol_init_internal(void)
{
    // Initialize protocol context
    memset(&g_protocol_context, 0, sizeof(protocol_context_t));
    
    // Initialize flash context using bootloader_protocol functions
    flash_context_init(&g_protocol_context.flash_ctx);
    
    // Set up Oracle-compatible configuration
    g_protocol_context.session_timeout_ms = 30000;  // 30 second timeout
    g_protocol_context.last_activity_time = get_tick_ms();
    g_protocol_context.state = PROTOCOL_STATE_IDLE;
    g_protocol_context.sequence_counter = 0;
    
    // Initialize transfer tracking
    g_protocol_context.data_received = false;
    g_protocol_context.expected_data_length = 0;
    g_protocol_context.actual_data_length = 0;
    
    // Initialize frame parser
    frame_parser_init(&g_frame_parser);
}

static bool vm_bootloader_protocol_process_uart_data(vm_bootloader_context_internal_t* ctx)
{
    bool frame_processed = false;
    bootloader_protocol_result_t parse_result = BOOTLOADER_PROTOCOL_SUCCESS;

    // Process all available UART data through frame parser
    while (uart_data_available()) {
        uint8_t byte = (uint8_t)uart_read_char();
        
        // Debug: Show byte being processed
        if (byte == 0x7E) diagnostic_char('S'); // START byte
        
        // Feed byte to frame parser
        parse_result = frame_parser_process_byte(&g_frame_parser, byte);
        
        if (parse_result == BOOTLOADER_PROTOCOL_SUCCESS) {
            // Update activity on successful byte processing
            vm_bootloader_protocol_update_activity();
            
            // Check if frame is complete
            if (frame_parser_is_complete(&g_frame_parser)) {
                // DEBUG: Frame complete marker
                diagnostic_char('G'); // Got complete frame
                
                // Process complete frame
                bootloader_protocol_result_t handle_result = 
                    vm_bootloader_protocol_handle_frame(ctx, &g_frame_parser.frame);
                
                if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
                    frame_processed = true;
                    diagnostic_char('H'); // Handle success
                } else {
                    diagnostic_char('I'); // Handle failed
                }
                
                // Reset parser for next frame
                frame_parser_reset(&g_frame_parser);
                
                // Update session state
                vm_bootloader_protocol_update_session_state(ctx);
                
                // Break out after processing complete frame
                break;
            }
            // Continue processing more bytes - don't treat success as error
        } else {
            // Frame parsing error occurred - break out and handle
            break;
        }
    }
    
    // Handle any parsing errors that occurred
    if (!frame_processed && parse_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        // Frame parsing error - reset and continue
        diagnostic_char('J'); // Parse error
        
        // Debug specific error type
        switch (parse_result) {
            case BOOTLOADER_PROTOCOL_ERROR_TIMEOUT:
                diagnostic_char('T');
                break;
            case BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE:
                diagnostic_char('L');
                break;
            case BOOTLOADER_PROTOCOL_ERROR_CRC_MISMATCH:
                diagnostic_char('C');
                break;
            case BOOTLOADER_PROTOCOL_ERROR_FRAME_INVALID:
                diagnostic_char('F');
                break;
            case BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID:
                diagnostic_char('S');
                break;
            default:
                diagnostic_char('U'); // Unknown error
                break;
        }
    }
    
    frame_parser_reset(&g_frame_parser);
    
    return frame_processed;
}

static bootloader_protocol_result_t vm_bootloader_protocol_handle_frame(vm_bootloader_context_internal_t* ctx, const bootloader_frame_t* frame)
{
    // Context IS needed for proper protocol state management
    if (!ctx) {
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Clear message structures for clean initialization
    memset(&g_current_request, 0, sizeof(g_current_request));
    memset(&g_current_response, 0, sizeof(g_current_response));
    
    // Decode protobuf request from frame payload
    pb_istream_t input_stream = pb_istream_from_buffer(frame->payload, frame->payload_length);
    
    // DEBUG: Add comprehensive protobuf decode diagnostics (Oracle-clean)
    diagnostic_char('P'); // Protobuf decode attempt
    
    // Debug: show payload length and first few bytes
    char len_debug = '0' + (frame->payload_length % 10);
    diagnostic_char(len_debug);
    
    // Show first 3 bytes of payload for pattern recognition
    if (frame->payload_length >= 3) {
        char byte1 = (frame->payload[0] < 32) ? '.' : frame->payload[0];
        char byte2 = (frame->payload[1] < 32) ? '.' : frame->payload[1]; 
        char byte3 = (frame->payload[2] < 32) ? '.' : frame->payload[2];
        diagnostic_char(byte1);
        diagnostic_char(byte2);
        diagnostic_char(byte3);
    }
    
    if (!pb_decode(&input_stream, BootloaderRequest_fields, &g_current_request)) {
        // Protobuf decode failed - enhanced diagnostics
        diagnostic_char('D'); // Decode failed marker
        g_protocol_context.state = PROTOCOL_STATE_ERROR;
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_DECODE;
    }
    
    // DEBUG: Show we successfully decoded the request
    diagnostic_char('!'); // Successful decode marker
    
    // DEBUG: Decode success markers
    diagnostic_char('R'); // pRotobuf decode success
    
    // Show which union field is set (critical for debugging)
    diagnostic_char('W'); // Which field marker
    diagnostic_char('0' + (char)g_current_request.which_request);
    
    // Handle the request using protocol handler
    diagnostic_char('@'); // About to call protocol handler
    bootloader_protocol_result_t handle_result = protocol_handle_request(&g_current_request, &g_current_response);
    diagnostic_char('#'); // Protocol handler returned
    
    if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
        // Send response back to Oracle
        return vm_bootloader_protocol_send_response(&g_current_response);
    }
    
    return handle_result;
}

static bootloader_protocol_result_t vm_bootloader_protocol_send_response(const BootloaderResponse* response)
{
    // Clear buffers for clean encoding
    memset(g_response_buffer, 0, sizeof(g_response_buffer));
    memset(g_request_buffer, 0, sizeof(g_request_buffer));
    
    // DEBUG: Send diagnostic markers about response construction (Oracle-clean)
    diagnostic_char('A'); // Response function called marker
    
    // Encode protobuf response
    pb_ostream_t output_stream = pb_ostream_from_buffer(g_response_buffer, sizeof(g_response_buffer));
    
    if (!pb_encode(&output_stream, BootloaderResponse_fields, response)) {
        diagnostic_char('B'); // Protobuf encode failed
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_ENCODE;
    }
    
    // DEBUG: Send protobuf encode success and size
    diagnostic_char('C'); // Protobuf encode success
    if (output_stream.bytes_written > 0) {
        diagnostic_char('D'); // Non-zero bytes written
    } else {
        diagnostic_char('Z'); // Zero bytes written!
    }
    
    // Frame the response - CRITICAL BUG FIX: Initialize frame_length to buffer size
    size_t frame_length = BOOTLOADER_MAX_FRAME_SIZE;  // Set to actual buffer size!
    bootloader_protocol_result_t frame_result = frame_encode(
        g_response_buffer, 
        output_stream.bytes_written,
        g_request_buffer,  // Reuse request buffer for framed response
        &frame_length
    );
    
    if (frame_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        diagnostic_char('E'); // Frame encode failed
        return frame_result;
    }
    
    // DEBUG: Frame encode success
    diagnostic_char('F'); // Frame encode success
    
    // Send framed response via UART
    for (size_t i = 0; i < frame_length; i++) {
        uart_write_char((char)g_request_buffer[i]);
    }
    
    // CRITICAL: Enable diagnostics after successful response transmission
    // This ensures Oracle gets clean handshake, then we enable debugging
    if (!g_diagnostics_enabled) {
        vm_bootloader_enable_diagnostics_after_handshake();
    }
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

static void vm_bootloader_protocol_update_session_state(vm_bootloader_context_internal_t* ctx)
{
    // Activate session on first protocol activity
    if (!ctx->session_active && g_protocol_context.state != PROTOCOL_STATE_IDLE) {
        ctx->session_active = true;
        ctx->session_start_ms = get_tick_ms();
    }
    
    // Update context state based on protocol state
    ctx->current_state = vm_bootloader_protocol_get_state();
}

// === COMPATIBILITY FUNCTIONS FOR OLD BOOTLOADER FRAMEWORK ===

void protocol_init(void)
{
    vm_bootloader_protocol_engine_init();
}

protocol_context_t* protocol_get_context(void)
{
    return vm_bootloader_protocol_get_context();
}

void protocol_update_activity(protocol_context_t* ctx)
{
    (void)ctx; // Use global context
    vm_bootloader_protocol_update_activity();
}

bootloader_protocol_result_t protocol_reset_session(protocol_context_t* ctx)
{
    (void)ctx; // Use global context
    vm_bootloader_protocol_reset_session();
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

void protocol_context_init(protocol_context_t* ctx)
{
    (void)ctx; // Use global context
    vm_bootloader_protocol_init_internal();
}

bool protocol_is_session_timeout(const protocol_context_t* ctx)
{
    (void)ctx; // Use global context
    return vm_bootloader_protocol_is_session_timeout();
}