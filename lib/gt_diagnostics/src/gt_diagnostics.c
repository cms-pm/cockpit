/*
 * Golden Triangle Diagnostics Framework Implementation
 * Phase 4.8: DIAG Integration for Golden Triangle Tests
 * 
 * Adapted from vm_bootloader/bootloader_diagnostics.c
 * Features:
 * - Timestamped structured logging via USART2  
 * - Modular output drivers
 * - Zero interference with Oracle protocol
 */

#include "gt_diagnostics.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef PLATFORM_STM32G4
#include "stm32g4xx_hal.h"
// External functions from vm_cockpit platform layer
extern HAL_StatusTypeDef stm32g4_debug_uart_init(uint32_t baud_rate);
extern HAL_StatusTypeDef stm32g4_debug_uart_transmit(uint8_t* data, uint16_t size);
extern uint32_t stm32g4_get_tick_ms(void);
#endif

// Prevent simultaneous operation with bootloader
#ifdef BOOTLOADER_RUNTIME_CONSOLE_ENABLED
    #error "Golden Triangle DIAG and Bootloader Runtime Console are mutually exclusive"
#endif

// =================================================================
// Global State
// =================================================================
static const diag_output_driver_t* g_output_driver = NULL;
static bool g_diagnostics_initialized = false;
static uint32_t g_init_timestamp = 0;

// =================================================================
// Level and Status Code Strings
// =================================================================
static const char* level_strings[] = {
    "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"
};

static const char* status_strings[] = {
    "SUCCESS", "ERR_GEN", "ERR_I2C", "ERR_UART", "ERR_GPIO", 
    "ERR_TOUT", "ERR_HW ", "ERR_TEST"
};

// =================================================================
// USART2 Output Driver Implementation (Golden Triangle Default)
// =================================================================
#ifdef PLATFORM_STM32G4
static bool gt_usart2_init(uint32_t baud_rate) {
    return (stm32g4_debug_uart_init(baud_rate) == HAL_OK);
}

static bool gt_usart2_write(const char* message, uint16_t length) {
    return (stm32g4_debug_uart_transmit((uint8_t*)message, length) == HAL_OK);
}

static void gt_usart2_flush(void) {
    // STM32 HAL handles flushing automatically
}

const diag_output_driver_t gt_diag_driver_usart2 = {
    .name = "GT_USART2",
    .init = gt_usart2_init,
    .write = gt_usart2_write,
    .flush = gt_usart2_flush
};
#endif

// =================================================================
// Core Implementation
// =================================================================

bool gt_diag_init(const diag_output_driver_t* driver, uint32_t baud_rate) {
    // Use default USART2 driver if none specified
    if (driver == NULL) {
#ifdef PLATFORM_STM32G4
        driver = &gt_diag_driver_usart2;
#else
        return false; // No default driver available
#endif
    }
    
    // Initialize the output driver - graceful degradation on failure
    if (!driver->init(baud_rate)) {
        // Continue without DIAG output as per requirement #3
        g_diagnostics_initialized = false;
        return false;
    }
    
    g_output_driver = driver;
    g_diagnostics_initialized = true;
    
#ifdef PLATFORM_STM32G4
    g_init_timestamp = stm32g4_get_tick_ms();
#else
    g_init_timestamp = 0;
#endif
    
    // Send initialization message
    char init_msg[256];
    snprintf(init_msg, sizeof(init_msg), 
            "\r\n=== Golden Triangle Diagnostics v4.8 ===\r\n"
            "Driver: %s @ %lu baud\r\n"
            "Format: [time] [level] [module] [file:line] [status] msg\r\n\r\n",
            driver->name, (unsigned long)baud_rate);
    
    g_output_driver->write(init_msg, strlen(init_msg));
    
    return true;
}

void gt_diag_log_full(log_level_t level, const char* module, const char* file, 
                      int line, status_code_t status, const char* format, ...) {
    if (!g_diagnostics_initialized || !g_output_driver) {
        return; // Graceful degradation - continue without output
    }
    
    // Extract filename from full path
    const char* filename = file;
    const char* last_slash = strrchr(file, '/');
    if (last_slash) {
        filename = last_slash + 1;
    }
    const char* last_backslash = strrchr(filename, '\\');
    if (last_backslash) {
        filename = last_backslash + 1;
    }
    
    // Get current timestamp (relative to initialization)
    uint32_t timestamp = 0;
#ifdef PLATFORM_STM32G4
    timestamp = stm32g4_get_tick_ms() - g_init_timestamp;
#endif
    
    // Build log message
    char log_buffer[512];
    int header_len = snprintf(log_buffer, sizeof(log_buffer),
                             "[%08lu] [%s] [%s] [%s:%d] [%s] ",
                             (unsigned long)timestamp,
                             level < 5 ? level_strings[level] : "UNKN ",
                             module ? module : "NULL",
                             filename,
                             line,
                             status < 8 ? status_strings[status] : "UNKN");
    
    if (header_len > 0 && header_len < (int)sizeof(log_buffer)) {
        // Add the formatted message
        va_list args;
        va_start(args, format);
        int msg_len = vsnprintf(log_buffer + header_len, 
                               sizeof(log_buffer) - header_len - 2, // Reserve space for \r\n
                               format, args);
        va_end(args);
        
        if (msg_len > 0) {
            int total_len = header_len + msg_len;
            if (total_len < (int)sizeof(log_buffer) - 2) {
                // Add newline
                log_buffer[total_len] = '\r';
                log_buffer[total_len + 1] = '\n';
                total_len += 2;
                
                // Send to output driver
                g_output_driver->write(log_buffer, total_len);
            }
        }
    }
}

void gt_diag_flow_step(char step, const char* description, status_code_t status) {
    if (!g_diagnostics_initialized) {
        return;
    }
    
    gt_diag_log_full(LOG_LEVEL_INFO, "GT_FLOW", __FILE__, __LINE__, status,
                     "Step %c: %s", step, description ? description : "");
}

void gt_diag_hex_dump(const char* label, const uint8_t* data, uint16_t length) {
    if (!g_diagnostics_initialized || !data || length == 0) {
        return;
    }
    
    gt_diag_log_full(LOG_LEVEL_DEBUG, "GT_HEXDUMP", __FILE__, __LINE__, STATUS_SUCCESS,
                     "%s (%u bytes):", label ? label : "Data", length);
    
    // Dump data in 16-byte rows
    char hex_line[128];
    for (uint16_t i = 0; i < length; i += 16) {
        int line_len = snprintf(hex_line, sizeof(hex_line), "  %04X: ", i);
        
        // Hex bytes
        uint16_t row_end = (i + 16 < length) ? i + 16 : length;
        for (uint16_t j = i; j < row_end; j++) {
            line_len += snprintf(hex_line + line_len, sizeof(hex_line) - line_len, 
                                "%02X ", data[j]);
        }
        
        // Padding for short rows
        for (uint16_t j = row_end; j < i + 16; j++) {
            line_len += snprintf(hex_line + line_len, sizeof(hex_line) - line_len, "   ");
        }
        
        // ASCII representation
        line_len += snprintf(hex_line + line_len, sizeof(hex_line) - line_len, " |");
        for (uint16_t j = i; j < row_end; j++) {
            char c = (data[j] >= 32 && data[j] <= 126) ? data[j] : '.';
            line_len += snprintf(hex_line + line_len, sizeof(hex_line) - line_len, "%c", c);
        }
        line_len += snprintf(hex_line + line_len, sizeof(hex_line) - line_len, "|\r\n");
        
        if (g_output_driver && line_len > 0) {
            g_output_driver->write(hex_line, line_len);
        }
    }
}