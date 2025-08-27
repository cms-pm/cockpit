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
#include "platform/platform_interface.h"

// External timing function
extern uint32_t get_tick_us(void);

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

// Message buffers for protobuf encoding/decoding (separated to prevent conflicts)



static uint8_t g_response_buffer[BOOTLOADER_MAX_FRAME_SIZE];  // Protobuf encoding
static uint8_t g_outbound_buffer[BOOTLOADER_MAX_FRAME_SIZE];  // Outbound framed responses  
static BootloaderRequest g_current_request;
static BootloaderResponse g_current_response;

// Forward declarations
static void vm_bootloader_protocol_init_internal(void);
static bool vm_bootloader_protocol_process_uart_data(vm_bootloader_context_internal_t* ctx);
static bootloader_protocol_result_t vm_bootloader_protocol_handle_frame(vm_bootloader_context_internal_t* ctx, const bootloader_frame_t* frame);
static bootloader_protocol_result_t vm_bootloader_protocol_send_response(const BootloaderResponse* response);
static void vm_bootloader_protocol_update_session_state(vm_bootloader_context_internal_t* ctx);



void vm_bootloader_enable_diagnostics_after_handshake(void) {
    // Disable diagnostic output completely to prevent Oracle frame corruption
    g_diagnostics_enabled = false;
}

// === PROTOCOL ENGINE API ===

void vm_bootloader_protocol_engine_init(void)
{
    if (!g_protocol_initialized) {
        // Initialize Oracle-style diagnostics first
        bootloader_diag_init(true);
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Protocol engine initializing");
        
        vm_bootloader_protocol_init_internal();
        g_protocol_initialized = true;
        
        DIAG_INFO(DIAG_COMPONENT_PROTOCOL_ENGINE, "Protocol engine initialization complete");
    }
}

protocol_context_t* vm_bootloader_protocol_get_context(void)
{
    if (!g_protocol_initialized) {
        vm_bootloader_protocol_engine_init();
    }
    return &g_protocol_context;
}

frame_parser_t* protocol_get_frame_parser(void)
{
    return &g_frame_parser;
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
    
    // Initialize protocol flow debug buffer
    memset(&g_protocol_context.flow_debug, 0, sizeof(protocol_flow_debug_t));
    g_protocol_context.flow_debug.step_count = 0;
    g_protocol_context.flow_debug.flow_complete = false;
    g_protocol_context.flow_debug.flow_start_time = 0;
    g_protocol_context.flow_debug.response_logged = false;
    
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
        
        // A-J Flow: Check for frame START byte (0x7E)
        if (byte == 0x7E && g_frame_parser.state == FRAME_STATE_IDLE) {
            DIAG_FLOW(DIAG_FLOW_A_FRAME_START, "Frame START received (0x7E)");
        }
        
        // Feed byte to frame parser
        parse_result = frame_parser_process_byte(&g_frame_parser, byte);
        
        if (parse_result == BOOTLOADER_PROTOCOL_SUCCESS) {
            // Update activity on successful byte processing
            vm_bootloader_protocol_update_activity();
            
            // A-J Flow: Track frame parsing progress
            if (g_frame_parser.state == FRAME_STATE_PAYLOAD && g_frame_parser.bytes_received == 2) {
                DIAG_FLOW(DIAG_FLOW_B_FRAME_LENGTH, "Frame length decoded");
            }
            
            // Check if frame is complete
            if (frame_parser_is_complete(&g_frame_parser)) {
                DIAG_FLOW(DIAG_FLOW_C_FRAME_PAYLOAD, "Frame payload received");
                DIAG_FLOW(DIAG_FLOW_D_FRAME_CRC_OK, "Frame CRC validated");
                
                // Process complete frame
                bootloader_protocol_result_t handle_result = vm_bootloader_protocol_handle_frame(ctx, &g_frame_parser.frame);
                
                if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
                    frame_processed = true;
                    DIAG_DEBUG(DIAG_COMPONENT_PROTOCOL_ENGINE, "Frame processed successfully");
                } else {
                    DIAG_ERROR(DIAG_COMPONENT_PROTOCOL_ENGINE, "Frame processing failed");
                }
                
                // Reset parser for next frame
                frame_parser_reset(&g_frame_parser);
                
                // Update session state
                vm_bootloader_protocol_update_session_state(ctx);
                
                // Break out after processing complete frame
                break;
            }
        } else {
            // Frame parsing error occurred - break out and handle
            DIAG_ERROR(DIAG_COMPONENT_FRAME_PARSER, "Frame parsing error");
            break;
        }
    }
    
    // Handle any parsing errors that occurred
    if (!frame_processed && parse_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        DIAG_WARN(DIAG_COMPONENT_FRAME_PARSER, "Frame parsing error - resetting parser");
        frame_parser_reset(&g_frame_parser);
    }
    
    return frame_processed;
}

static bootloader_protocol_result_t vm_bootloader_protocol_handle_frame(vm_bootloader_context_internal_t* ctx, const bootloader_frame_t* frame)
{
    // Context IS needed for proper protocol state management
    if (!ctx) {
        DIAG_ERROR(DIAG_COMPONENT_PROTOCOL_ENGINE, "Invalid context in handle_frame");
        return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    }
    
    // Log frame payload for debugging
    DIAG_BUFFER(DIAG_LEVEL_DEBUG, DIAG_COMPONENT_PROTOCOL_ENGINE, "Frame payload", frame->payload, frame->payload_length);
    
    // Clear message structures for clean initialization
    memset(&g_current_request, 0, sizeof(g_current_request));
    memset(&g_current_response, 0, sizeof(g_current_response));
    
    // Buffer boundary checking
    if (frame->payload_length > BOOTLOADER_MAX_PAYLOAD_SIZE) {
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "Payload too large: %u > %u", 
                   (unsigned int)frame->payload_length, BOOTLOADER_MAX_PAYLOAD_SIZE);
        return BOOTLOADER_PROTOCOL_ERROR_PAYLOAD_TOO_LARGE;
    }
    
    // E: Protobuf decode initiated
    DIAG_FLOW(DIAG_FLOW_E_PROTOBUF_DECODE_START, "Starting protobuf decode");
    pb_istream_t input_stream = pb_istream_from_buffer(frame->payload, frame->payload_length);
    
    if (!pb_decode(&input_stream, BootloaderRequest_fields, &g_current_request)) {
        // Protobuf decode failed - log more details
        DIAG_ERRORF(DIAG_COMPONENT_NANOPB_DECODE, "Protobuf decode failed, payload_length=%u", 
                   (unsigned int)frame->payload_length);
        g_protocol_context.state = PROTOCOL_STATE_ERROR;
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_DECODE;
    }
    
    // F: Protobuf decode completed successfully
    DIAG_FLOW(DIAG_FLOW_F_PROTOBUF_DECODE_OK, "Protobuf decode success");
    
    // G: Message processing started
    DIAG_FLOW(DIAG_FLOW_G_MESSAGE_PROCESSING, "Starting message processing");
    bootloader_protocol_result_t handle_result = protocol_handle_request(&g_current_request, &g_current_response);
    
    if (handle_result == BOOTLOADER_PROTOCOL_SUCCESS) {
        // H: Response generation started
        DIAG_FLOW(DIAG_FLOW_H_RESPONSE_GENERATION, "Response generation success");
        
        // Send response back to Oracle
        bootloader_protocol_result_t send_result = vm_bootloader_protocol_send_response(&g_current_response);
        return send_result;
    } else {
        DIAG_ERROR(DIAG_COMPONENT_MESSAGE_HANDLER, "Message processing failed");
        return handle_result;
    }
}

static bootloader_protocol_result_t vm_bootloader_protocol_send_response(const BootloaderResponse* response)
{
    // Clear buffers for clean encoding
    memset(g_response_buffer, 0, sizeof(g_response_buffer));
    memset(g_outbound_buffer, 0, sizeof(g_outbound_buffer));
    
    // Encode protobuf response
    pb_ostream_t output_stream = pb_ostream_from_buffer(g_response_buffer, sizeof(g_response_buffer));
    
    if (!pb_encode(&output_stream, BootloaderResponse_fields, response)) {
        DIAG_ERRORF(DIAG_COMPONENT_NANOPB_ENCODE, "Protobuf encode failed, buffer_size=%u", 
                   (unsigned int)sizeof(g_response_buffer));
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_ENCODE;
    }
    
    // Buffer boundary checking for encoded data
    if (output_stream.bytes_written > sizeof(g_response_buffer)) {
        DIAG_ERRORF(DIAG_COMPONENT_NANOPB_ENCODE, "Encode buffer overflow: %u > %u", 
                   (unsigned int)output_stream.bytes_written, (unsigned int)sizeof(g_response_buffer));
        return BOOTLOADER_PROTOCOL_ERROR_PROTOBUF_ENCODE;
    }
    
    // I: Response encode completed
    DIAG_FLOW(DIAG_FLOW_I_RESPONSE_ENCODE_OK, "Response protobuf encode success");
    DIAG_DEBUGF(DIAG_COMPONENT_NANOPB_ENCODE, "Encoded %u bytes", (unsigned int)output_stream.bytes_written);
    
    // Frame the response - Initialize frame_length to buffer size
    size_t frame_length = BOOTLOADER_MAX_FRAME_SIZE;
    bootloader_protocol_result_t frame_result = frame_encode(
        g_response_buffer, 
        output_stream.bytes_written,
        g_outbound_buffer,  // Use dedicated outbound buffer
        &frame_length
    );
    
    if (frame_result != BOOTLOADER_PROTOCOL_SUCCESS) {
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "Frame encoding failed, result=%d", frame_result);
        return frame_result;
    }
    
    // Buffer boundary checking for framed data
    if (frame_length > BOOTLOADER_MAX_FRAME_SIZE) {
        DIAG_ERRORF(DIAG_COMPONENT_PROTOCOL_ENGINE, "Frame too large: %u > %u", 
                   (unsigned int)frame_length, BOOTLOADER_MAX_FRAME_SIZE);
        return BOOTLOADER_PROTOCOL_ERROR_FRAME_INVALID;
    }
    
    // Log outbound frame for debugging
    DIAG_BUFFER(DIAG_LEVEL_DEBUG, DIAG_COMPONENT_PROTOCOL_ENGINE, "Outbound frame", g_outbound_buffer, frame_length);
    
    // Send framed response via UART - ATOMIC TRANSMISSION with dedicated buffer
    platform_uart_transmit((const uint8_t*)g_outbound_buffer, (uint16_t)frame_length);
    
    // J: Response transmitted
    DIAG_FLOW(DIAG_FLOW_J_RESPONSE_TRANSMITTED, "Response transmitted successfully");
    
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

// === PROTOCOL FLOW DEBUG FUNCTIONS ===

void protocol_flow_log_step(char step) 
{
    if (!step) {
        return;
    }
    // if (!g_protocol_initialized) {
    //     return;
    // }
    
    // protocol_flow_debug_t* flow = &g_protocol_context.flow_debug;
    // uint32_t current_time = get_tick_us();
    
    // // Reset flow buffer on 'A' (start of new flow)
    // if (step == 'A') {
    //     flow->step_count = 0;
    //     flow->flow_complete = false;
    //     flow->flow_start_time = current_time;
    //     flow->response_logged = false;
    // }
    
    // // Add step to buffer if space available
    // if (flow->step_count < PROTOCOL_FLOW_BUFFER_SIZE - 1) {
    //     flow->flow_steps[flow->step_count] = step;
    //     flow->step_timestamps[flow->step_count] = current_time;
    //     flow->step_count++;
    //     flow->flow_steps[flow->step_count] = '\0'; // Null terminate
    // }
    
    // // Mark complete on success steps
    // if (step == '7' || step == 'H') {
    //     flow->flow_complete = true;
    // }
}

void protocol_flow_debug_dump(void)
{
    if (!g_protocol_initialized) {
        return;
    }
    
    protocol_flow_debug_t* flow = &g_protocol_context.flow_debug;
    
    uart_write_string("=== PROTOCOL FLOW DEBUG ===\r\n");
    
    if (flow->step_count > 0) {
        uart_write_string("Flow steps: ");
        uart_write_string(flow->flow_steps);
        uart_write_string("\r\n");
        
        // Show timing analysis - focus on ST→CD transition
        uart_write_string("CRITICAL TIMING ANALYSIS:\r\n");
        for (uint8_t i = 0; i < flow->step_count; i++) {
            char step = flow->flow_steps[i];
            uint32_t delta_ms = (flow->step_timestamps[i] - flow->flow_start_time) / 1000;
            
            // Focus on key transitions
            if (step == 'S' || step == 'T' || step == 'C' || step == 'D') {
                uart_write_string("  ");
                uart_write_char(step);
                uart_write_string("@");
                
                // Simple decimal output for timing
                if (delta_ms < 10) {
                    uart_write_char('0' + delta_ms);
                } else if (delta_ms < 100) {
                    uart_write_char('0' + (delta_ms / 10));
                    uart_write_char('0' + (delta_ms % 10));
                } else {
                    uart_write_string("99+");
                }
                uart_write_string("ms ");
            }
        }
        uart_write_string("\r\n");
        
        // Show response hex for bit stuffing analysis
        if (flow->response_logged) {
            uart_write_string("RESPONSE HEX SENT: ");
            for (uint8_t i = 0; i < flow->response_length; i++) {
                uint8_t byte = flow->response_hex[i];
                // Simple hex output
                uint8_t high = (byte >> 4) & 0x0F;
                uint8_t low = byte & 0x0F;
                uart_write_char((high < 10) ? ('0' + high) : ('A' + high - 10));
                uart_write_char((low < 10) ? ('0' + low) : ('A' + low - 10));
                uart_write_char(' ');
            }
            uart_write_string("\r\n");
        }
        
        uart_write_string("Step meanings:\r\n");
        uart_write_string("  A=Frame ready, B=Call handle, C=Handle returned\r\n");
        uart_write_string("  D=Handle success, E=Handle failed\r\n");
        uart_write_string("  F=Protobuf decode start, G=Decode failed, H=Decode success\r\n");
        uart_write_string("  I=Protocol handler start, J=Handler returned, K=Handler failed\r\n");
        uart_write_string("  L=Response start, M=Response encode, N=Encode failed, O=Encode success\r\n");
        uart_write_string("  P=Frame encode, Q=Frame failed, R=Frame success, S=UART start, T=UART done\r\n");
        uart_write_string("  U=About to check handle_result, V=handle_frame ERROR, W=handle_frame SUCCESS\r\n");
        uart_write_string("  Y=About to return ERROR, Z=About to return SUCCESS\r\n");
        uart_write_string("  X=Oracle sent new data during CD phase (SMOKING GUN!)\r\n");
    } else {
        uart_write_string("No flow steps recorded\r\n");
    }
    
    uart_write_string("===========================\r\n");
}

void protocol_flow_reset(void)
{
    if (!g_protocol_initialized) {
        return;
    }
    
    memset(&g_protocol_context.flow_debug, 0, sizeof(protocol_flow_debug_t));
    g_protocol_context.flow_debug.step_count = 0;
    g_protocol_context.flow_debug.flow_complete = false;
}
