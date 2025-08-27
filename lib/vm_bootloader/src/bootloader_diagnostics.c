/*
 * CockpitVM Bootloader Diagnostics - Implementation
 * 
 * Oracle-style diagnostic logging system outputting to USART2 for
 * Oracle-clean debugging without interference with USART1 protocol.
 */

#include "bootloader_diagnostics.h"
#include "host_interface/host_interface.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// External timing function for microsecond timestamps
extern uint32_t get_tick_us(void);

// Diagnostic system state
static bool g_diag_enabled = false;
static bool g_diag_initialized = false;
static bootloader_diag_level_t g_diag_level = DIAG_LEVEL_INFO;
static uint32_t g_session_start_time_us = 0;

// Component name strings for Oracle-style output
static const char* component_names[] = {
    "PROTOCOL_ENGINE",
    "FRAME_PARSER",
    "NANOPB_DECODE",
    "NANOPB_ENCODE", 
    "MESSAGE_HANDLER",
    "FLASH_PROGRAMMER",
    "VM_BOOTLOADER",
    "CONTEXT_MANAGER"
};

// Level name strings for Oracle-style output
static const char* level_names[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};

// =================================================================
// Internal Helper Functions
// =================================================================

static void diag_raw_output(const char* str) {
    if (!g_diag_enabled || !g_diag_initialized) {
        return;
    }
    
    debug_uart_write_string(str);
}

static uint32_t diag_get_timestamp_us(void) {
    uint32_t current_time = get_tick_us();
    if (g_session_start_time_us == 0) {
        g_session_start_time_us = current_time;
        return 0;
    }
    return current_time - g_session_start_time_us;
}

// =================================================================
// Public API Implementation
// =================================================================

bool bootloader_diag_init(bool enable_output) {
    // Initialize debug UART (USART2) at 115200 baud
    debug_uart_begin(115200);
    
    g_diag_enabled = enable_output;
    g_diag_initialized = true;
    g_session_start_time_us = 0; // Reset on next timestamp call
    
    if (enable_output) {
        // Output Oracle-style initialization message
        diag_raw_output("\r\n=== CockpitVM Bootloader Diagnostics Started ===\r\n");
        diag_raw_output("Format: [timestamp_us] LEVEL COMPONENT: message\r\n");
        diag_raw_output("A-J Flow: Frame processing steps A through J\r\n");
        diag_raw_output("Debug Channel: USART2 PA2/PA3 @ 115200 8N1\r\n");
        diag_raw_output("Protocol Channel: USART1 PA9/PA10 @ 115200 8N1\r\n");
        diag_raw_output("=================================================\r\n\r\n");
    }
    
    return true;
}

void bootloader_diag_set_level(bootloader_diag_level_t level) {
    g_diag_level = level;
}

void bootloader_diag_set_enabled(bool enabled) {
    g_diag_enabled = enabled;
}

void bootloader_diag_log(bootloader_diag_level_t level, 
                        bootloader_diag_component_t component,
                        const char* message) {
    if (!g_diag_enabled || !g_diag_initialized || level > g_diag_level) {
        return;
    }
    
    // Format: [timestamp_us] LEVEL COMPONENT: message
    char formatted_msg[256];
    uint32_t timestamp = diag_get_timestamp_us();
    
    snprintf(formatted_msg, sizeof(formatted_msg), 
             "[%08lu] %s %s: %s\r\n",
             timestamp,
             level_names[level],
             component_names[component],
             message ? message : "NULL");
    
    diag_raw_output(formatted_msg);
}

void bootloader_diag_logf(bootloader_diag_level_t level, 
                         bootloader_diag_component_t component,
                         const char* format, ...) {
    if (!g_diag_enabled || !g_diag_initialized || level > g_diag_level) {
        return;
    }
    
    char message_buffer[192];
    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);
    
    bootloader_diag_log(level, component, message_buffer);
}

void bootloader_diag_flow_step(bootloader_diag_flow_step_t flow_step, 
                              const char* context) {
    if (!g_diag_enabled || !g_diag_initialized) {
        return;
    }
    
    // Oracle-style A-J flow output: [timestamp] FLOW_X: context
    char flow_msg[128];
    uint32_t timestamp = diag_get_timestamp_us();
    
    snprintf(flow_msg, sizeof(flow_msg),
             "[%08lu] FLOW_%c: %s\r\n",
             timestamp,
             (char)flow_step,
             context ? context : "");
    
    diag_raw_output(flow_msg);
}

void bootloader_diag_log_buffer(bootloader_diag_level_t level,
                               bootloader_diag_component_t component,
                               const char* label,
                               const uint8_t* buffer,
                               size_t length) {
    if (!g_diag_enabled || !g_diag_initialized || level > g_diag_level) {
        return;
    }
    
    if (buffer == NULL || length == 0) {
        bootloader_diag_logf(level, component, "%s: NULL or empty buffer", label ? label : "BUFFER");
        return;
    }
    
    // Oracle-style hex dump with header
    bootloader_diag_logf(level, component, "%s (%u bytes):", label ? label : "BUFFER", (unsigned int)length);
    
    // Hex output in 16-byte lines
    char hex_line[64];
    for (size_t i = 0; i < length; i += 16) {
        char hex_bytes[64] = {0};
        size_t line_length = (length - i > 16) ? 16 : (length - i);
        
        for (size_t j = 0; j < line_length; j++) {
            snprintf(hex_bytes + (j * 3), 4, "%02X ", buffer[i + j]);
        }
        
        snprintf(hex_line, sizeof(hex_line), "  %04X: %s", (unsigned int)i, hex_bytes);
        diag_raw_output(hex_line);
        diag_raw_output("\r\n");
    }
}