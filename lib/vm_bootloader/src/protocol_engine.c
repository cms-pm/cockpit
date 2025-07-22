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

// Global protocol context and frame parser
static protocol_context_t g_protocol_context;
static frame_parser_t g_frame_parser;
static bool g_protocol_initialized = false;

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
    
    // Process all available UART data through frame parser
    while (uart_data_available()) {
        uint8_t byte = (uint8_t)uart_read_char();
        
        // Feed byte to frame parser
        bootloader_protocol_result_t parse_result = frame_parser_process_byte(&g_frame_parser, byte);
        
        if (parse_result == BOOTLOADER_PROTOCOL_SUCCESS) {
            // Update activity on successful byte processing
            vm_bootloader_protocol_update_activity();
            
            // Check if frame is complete
            if (frame_parser_is_complete(&g_frame_parser)) {
                // Process complete frame
                bootloader_protocol_result_t handle_result = 
                    vm_bootloader_protocol_handle_frame(ctx, &g_frame_parser.frame);
                
                if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
                    frame_processed = true;
                }
                
                // Reset parser for next frame
                frame_parser_reset(&g_frame_parser);
                
                // Update session state
                vm_bootloader_protocol_update_session_state(ctx);
            }
        } else if (parse_result != BOOTLOADER_PROTOCOL_SUCCESS) {
            // Frame parsing error - reset and continue
            frame_parser_reset(&g_frame_parser);
        }
    }
    
    return frame_processed;
}

static bootloader_protocol_result_t vm_bootloader_protocol_handle_frame(vm_bootloader_context_internal_t* ctx, const bootloader_frame_t* frame)
{
    (void)ctx; // Context not needed for frame handling
    
    // Decode protobuf request from frame payload
    pb_istream_t input_stream = pb_istream_from_buffer(frame->payload, frame->payload_length);
    
    if (!pb_decode(&input_stream, BootloaderRequest_fields, &g_current_request)) {
        // Protobuf decode failed
        g_protocol_context.state = PROTOCOL_STATE_ERROR;
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_DECODE;
    }
    
    // Handle the request using protocol handler
    bootloader_protocol_result_t handle_result = protocol_handle_request(&g_current_request, &g_current_response);
    
    if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
        // Send response back to Oracle
        return vm_bootloader_protocol_send_response(&g_current_response);
    }
    
    return handle_result;
}

static bootloader_protocol_result_t vm_bootloader_protocol_send_response(const BootloaderResponse* response)
{
    // Encode protobuf response
    pb_ostream_t output_stream = pb_ostream_from_buffer(g_response_buffer, sizeof(g_response_buffer));
    
    if (!pb_encode(&output_stream, BootloaderResponse_fields, response)) {
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_ENCODE;
    }
    
    // Frame the response
    size_t frame_length = 0;
    bootloader_protocol_result_t frame_result = frame_encode(
        g_response_buffer, 
        output_stream.bytes_written,
        g_request_buffer,  // Reuse request buffer for framed response
        &frame_length
    );
    
    if (frame_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        return frame_result;
    }
    
    // Send framed response via UART
    for (size_t i = 0; i < frame_length; i++) {
        uart_write_char((char)g_request_buffer[i]);
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