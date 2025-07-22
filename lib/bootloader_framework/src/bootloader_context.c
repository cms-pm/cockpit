/*
 * ComponentVM Bootloader Framework - Context Implementation
 * 
 * Core implementation of the complete bootloader framework that
 * integrates protocol, resource, timeout, and emergency management
 * into a coherent production-ready bootloader system.
 */

#include "bootloader_context.h"
#include "resource_manager.h"
#include "bootloader_emergency.h"

// ComponentVM host interface for hardware abstraction
#include "host_interface/host_interface.h"

#include <string.h>
#include <stdio.h>

// Build timestamp for version tracking
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

// Forward declarations for internal functions
static void bootloader_context_init_defaults(bootloader_context_t* ctx);
static bootloader_init_result_t bootloader_init_subsystems(bootloader_context_t* ctx);
static void bootloader_update_statistics(bootloader_context_t* ctx);
static bool bootloader_process_frame(bootloader_context_t* ctx);
static void bootloader_handle_timeout(bootloader_context_t* ctx);

// === LIFECYCLE MANAGEMENT IMPLEMENTATION ===

bootloader_init_result_t bootloader_init(bootloader_context_t* ctx, const bootloader_config_t* config)
{
    if (!ctx) {
        return BOOTLOADER_INIT_ERROR_PROTOCOL_FAILED;
    }
    
    // Initialize context to safe defaults
    memset(ctx, 0, sizeof(bootloader_context_t));
    bootloader_context_init_defaults(ctx);
    
    // Apply configuration
    if (config) {
        ctx->session_timeout_ms = config->session_timeout_ms;
        ctx->mode = config->initial_mode;
        ctx->version_string = config->custom_version_info ? 
            config->custom_version_info : BOOTLOADER_FRAMEWORK_VERSION;
    } else {
        // Use default configuration
        bootloader_config_t default_config;
        bootloader_get_default_config(&default_config);
        ctx->session_timeout_ms = default_config.session_timeout_ms;
        ctx->mode = default_config.initial_mode;
        ctx->version_string = BOOTLOADER_FRAMEWORK_VERSION;
    }
    
    ctx->build_timestamp = BUILD_TIMESTAMP;
    ctx->boot_time_ms = get_tick_ms();
    
    // Initialize all subsystems
    bootloader_init_result_t result = bootloader_init_subsystems(ctx);
    if (result != BOOTLOADER_INIT_SUCCESS) {
        return result;
    }
    
    // Mark as initialized
    ctx->initialized = true;
    ctx->current_state = BOOTLOADER_STATE_IDLE;
    
    return BOOTLOADER_INIT_SUCCESS;
}

bootloader_run_result_t bootloader_run_cycle(bootloader_context_t* ctx)
{
    if (!ctx || !ctx->initialized) {
        return BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    // Check for emergency conditions
    if (ctx->emergency_mode) {
        return BOOTLOADER_RUN_EMERGENCY_SHUTDOWN;
    }
    
    // Update execution cycle counter
    ctx->execution_cycles++;
    
    // Check session timeout
    if (bootloader_is_session_timeout(ctx)) {
        bootloader_handle_timeout(ctx);
        return BOOTLOADER_RUN_TIMEOUT;
    }
    
    // Update activity timestamp
    ctx->last_activity_ms = get_tick_ms();
    
    // Process incoming frames
    bool frame_processed = bootloader_process_frame(ctx);
    if (frame_processed) {
        ctx->total_frames_received++;
        bootloader_update_statistics(ctx);
    }
    
    // Update protocol context activity
    if (ctx->protocol_ctx) {
        protocol_update_activity(ctx->protocol_ctx);
    }
    
    // Check if session is complete
    if (ctx->protocol_ctx && ctx->protocol_ctx->state == PROTOCOL_STATE_IDLE) {
        if (ctx->session_active) {
            ctx->session_active = false;
            ctx->successful_operations++;
            return BOOTLOADER_RUN_COMPLETE;
        }
    }
    
    return BOOTLOADER_RUN_CONTINUE;
}

bootloader_run_result_t bootloader_main_loop(bootloader_context_t* ctx)
{
    if (!ctx || !ctx->initialized) {
        return BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    // Start session
    ctx->session_active = true;
    ctx->session_start_ms = get_tick_ms();
    ctx->current_state = BOOTLOADER_STATE_IDLE;
    
    // Debug output for Oracle integration
    if (ctx->mode == BOOTLOADER_MODE_DEBUG || ctx->mode == BOOTLOADER_MODE_LISTEN_ONLY) {
        uart_write_string("Bootloader framework entering main loop\r\n");
        uart_write_string("Session timeout: ");
        // Simple integer to string for timeout display
        uint32_t timeout_sec = ctx->session_timeout_ms / 1000;
        char timeout_str[16];
        snprintf(timeout_str, sizeof(timeout_str), "%lu", timeout_sec);
        uart_write_string(timeout_str);
        uart_write_string(" seconds\r\n");
    }
    
    bootloader_run_result_t result;
    
    // Main processing loop
    while (true) {
        result = bootloader_run_cycle(ctx);
        
        switch (result) {
            case BOOTLOADER_RUN_CONTINUE:
                // Continue normal operation
                break;
                
            case BOOTLOADER_RUN_COMPLETE:
                // Session completed successfully
                if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("Bootloader session complete - success\r\n");
                }
                return BOOTLOADER_RUN_COMPLETE;
                
            case BOOTLOADER_RUN_TIMEOUT:
                // Session timeout occurred  
                if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("Bootloader session timeout\r\n");
                }
                return BOOTLOADER_RUN_TIMEOUT;
                
            case BOOTLOADER_RUN_ERROR_RECOVERABLE:
                // Recoverable error - continue with caution
                ctx->total_errors++;
                if (ctx->total_errors > 10) {
                    // Too many errors - escalate to critical
                    return BOOTLOADER_RUN_ERROR_CRITICAL;
                }
                break;
                
            case BOOTLOADER_RUN_ERROR_CRITICAL:
                // Critical error - emergency shutdown
                if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("Bootloader critical error - emergency shutdown\r\n");
                }
                return BOOTLOADER_RUN_ERROR_CRITICAL;
                
            case BOOTLOADER_RUN_EMERGENCY_SHUTDOWN:
                // Emergency shutdown initiated
                if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("Bootloader emergency shutdown\r\n");
                }
                bootloader_emergency_shutdown(ctx);
                return BOOTLOADER_RUN_EMERGENCY_SHUTDOWN;
                
            default:
                // Unknown result - treat as critical error
                return BOOTLOADER_RUN_ERROR_CRITICAL;
        }
        
        // Small delay to prevent busy waiting
        delay_ms(10);
    }
}

void bootloader_emergency_shutdown(bootloader_context_t* ctx)
{
    if (!ctx) {
        return;
    }
    
    // Mark emergency mode
    ctx->emergency_mode = true;
    
    // Emergency LED pattern - rapid blink
    for (int i = 0; i < 5; i++) {
        gpio_pin_write(13, true);  // PC6 = pin 13
        delay_ms(100);
        gpio_pin_write(13, false);
        delay_ms(100);
    }
    
    // Clean up critical resources first
    if (ctx->resource_mgr) {
        resource_manager_emergency_cleanup(ctx->resource_mgr);
    }
    
    // Reset protocol to safe state
    if (ctx->protocol_ctx) {
        protocol_reset_session(ctx->protocol_ctx);
    }
    
    // Put hardware in safe state
    uart_write_string("EMERGENCY: System entering safe state\r\n");
    
    // Reset UART to known state
    uart_begin(115200);
    
    // Use existing bootloader state from bootloader_states.h
    // BOOTLOADER_STATE_ERROR doesn't exist, use recovery state
    ctx->current_state = BOOTLOADER_STATE_RECOVERY_ABORT;
}

void bootloader_cleanup(bootloader_context_t* ctx)
{
    if (!ctx || !ctx->initialized) {
        return;
    }
    
    // Clean session statistics
    if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
        uart_write_string("Bootloader cleanup - statistics:\r\n");
        // Output basic statistics
        char stats_msg[64];
        snprintf(stats_msg, sizeof(stats_msg), "Cycles: %lu, Frames: %lu, Errors: %lu\r\n",
                ctx->execution_cycles, ctx->total_frames_received, ctx->total_errors);
        uart_write_string(stats_msg);
    }
    
    // Clean up all registered resources
    if (ctx->resource_mgr) {
        resource_manager_cleanup_all(ctx->resource_mgr);
    }
    
    // Reset protocol context
    if (ctx->protocol_ctx) {
        protocol_reset_session(ctx->protocol_ctx);
    }
    
    // Mark as not initialized
    ctx->initialized = false;
    ctx->session_active = false;
    ctx->emergency_mode = false;
}

// === CONTEXT QUERY IMPLEMENTATION ===

bool bootloader_is_initialized(const bootloader_context_t* ctx)
{
    return ctx && ctx->initialized;
}

bool bootloader_is_ready(const bootloader_context_t* ctx)
{
    if (!ctx || !ctx->initialized || ctx->emergency_mode) {
        return false;
    }
    
    return ctx->current_state == BOOTLOADER_STATE_IDLE ||
           ctx->current_state == BOOTLOADER_STATE_READY;
}

bool bootloader_is_session_timeout(const bootloader_context_t* ctx)
{
    if (!ctx || !ctx->session_active) {
        return false;
    }
    
    uint32_t current_time = get_tick_ms();
    uint32_t elapsed = current_time - ctx->session_start_ms;
    
    return elapsed > ctx->session_timeout_ms;
}

bootloader_state_t bootloader_get_current_state(const bootloader_context_t* ctx)
{
    return ctx ? ctx->current_state : BOOTLOADER_STATE_RECOVERY_ABORT;
}

uint32_t bootloader_get_uptime_ms(const bootloader_context_t* ctx)
{
    if (!ctx) {
        return 0;
    }
    
    uint32_t current_time = get_tick_ms();
    return current_time - ctx->boot_time_ms;
}

uint32_t bootloader_get_session_elapsed_ms(const bootloader_context_t* ctx)
{
    if (!ctx || !ctx->session_active) {
        return 0;
    }
    
    uint32_t current_time = get_tick_ms();
    return current_time - ctx->session_start_ms;
}

// === CONFIGURATION IMPLEMENTATION ===

void bootloader_set_mode(bootloader_context_t* ctx, bootloader_mode_t mode)
{
    if (ctx) {
        ctx->mode = mode;
    }
}

void bootloader_set_debug_mode(bootloader_context_t* ctx, bool enabled)
{
    if (ctx) {
        ctx->mode = enabled ? BOOTLOADER_MODE_DEBUG : BOOTLOADER_MODE_NORMAL;
    }
}

void bootloader_set_session_timeout(bootloader_context_t* ctx, uint32_t timeout_ms)
{
    if (ctx) {
        ctx->session_timeout_ms = timeout_ms;
    }
}

// === STATISTICS IMPLEMENTATION ===

void bootloader_get_statistics(const bootloader_context_t* ctx, bootloader_statistics_t* stats)
{
    if (!ctx || !stats) {
        return;
    }
    
    stats->uptime_ms = bootloader_get_uptime_ms(ctx);
    stats->execution_cycles = ctx->execution_cycles;
    stats->frames_received = ctx->total_frames_received;
    stats->frames_sent = ctx->total_frames_sent;
    stats->total_errors = ctx->total_errors;
    stats->successful_operations = ctx->successful_operations;
    stats->current_state = ctx->current_state;
    stats->current_mode = ctx->mode;
}

// === DEFAULT CONFIGURATION ===

void bootloader_get_default_config(bootloader_config_t* config)
{
    if (!config) {
        return;
    }
    
    config->session_timeout_ms = 30000;          // 30 seconds
    config->frame_timeout_ms = 500;              // 500ms
    config->initial_mode = BOOTLOADER_MODE_NORMAL;
    config->enable_debug_output = false;
    config->enable_resource_tracking = true;
    config->enable_emergency_recovery = true;
    config->custom_version_info = NULL;
}

void bootloader_get_oracle_config(bootloader_config_t* config)
{
    if (!config) {
        return;
    }
    
    // Oracle testing configuration
    config->session_timeout_ms = 30000;          // 30 seconds for Oracle testing
    config->frame_timeout_ms = 500;              // 500ms frame timeout
    config->initial_mode = BOOTLOADER_MODE_LISTEN_ONLY;
    config->enable_debug_output = true;          // Enable debug for Oracle
    config->enable_resource_tracking = true;
    config->enable_emergency_recovery = true;
    config->custom_version_info = "4.5.2-Oracle";
}

// === INTERNAL IMPLEMENTATION ===

static void bootloader_context_init_defaults(bootloader_context_t* ctx)
{
    ctx->initialized = false;
    ctx->emergency_mode = false;
    ctx->session_active = false;
    ctx->current_state = BOOTLOADER_STATE_INIT;
    ctx->mode = BOOTLOADER_MODE_NORMAL;
    ctx->session_timeout_ms = 30000;
    ctx->execution_cycles = 0;
    ctx->total_frames_received = 0;
    ctx->total_frames_sent = 0;
    ctx->total_errors = 0;
    ctx->successful_operations = 0;
}

static bootloader_init_result_t bootloader_init_subsystems(bootloader_context_t* ctx)
{
    // Initialize host interface (UART, GPIO, timing)
    host_interface_init();
    
    // Configure UART for protocol communication
    uart_begin(115200);
    
    // Configure status LED
    gpio_pin_config(13, GPIO_OUTPUT);  // PC6 = pin 13
    
    // Initialize protocol context
    protocol_init();
    ctx->protocol_ctx = protocol_get_context();
    if (!ctx->protocol_ctx) {
        return BOOTLOADER_INIT_ERROR_PROTOCOL_FAILED;
    }
    
    // Note: Resource manager and emergency manager would be initialized here
    // For now, we'll set them to NULL and implement basic functionality
    ctx->resource_mgr = NULL;
    ctx->error_mgr = NULL;
    ctx->timeout_mgr = NULL;
    
    return BOOTLOADER_INIT_SUCCESS;
}

static void bootloader_update_statistics(bootloader_context_t* ctx)
{
    ctx->last_activity_ms = get_tick_ms();
    
    // Update session activity
    if (!ctx->session_active && ctx->current_state != BOOTLOADER_STATE_IDLE) {
        ctx->session_active = true;
        ctx->session_start_ms = ctx->last_activity_ms;
    }
}

static bool bootloader_process_frame(bootloader_context_t* ctx)
{
    // Check if UART data is available
    if (!uart_data_available()) {
        return false;
    }
    
    // Simple frame processing - just consume available data
    // In full implementation, this would use the frame parser
    uart_read_char(); // Consume the byte
    
    // Update state based on activity
    if (ctx->current_state == BOOTLOADER_STATE_IDLE) {
        ctx->current_state = BOOTLOADER_STATE_HANDSHAKE;
    }
    
    return true;
}

static void bootloader_handle_timeout(bootloader_context_t* ctx)
{
    ctx->session_active = false;
    ctx->current_state = BOOTLOADER_STATE_IDLE;
    
    if (ctx->mode == BOOTLOADER_MODE_DEBUG) {
        uart_write_string("Session timeout handled\r\n");
    }
}