/*
 * CockpitVM Unified Bootloader - Main Implementation
 * 
 * Core implementation consolidating bootloader framework, protocol engine,
 * and state management into a coherent embedded-first bootloader system.
 * Optimized for STM32G431CB and Oracle testing integration.
 */

#include "vm_bootloader.h"
#include "context_internal.h"
#include "bootloader_protocol.h"

// CockpitVM host interface for hardware abstraction
#include "host_interface/host_interface.h"
// Bootloader states integration
#include "bootloader_states.h"

// Protocol engine integration
extern void vm_bootloader_protocol_engine_init(void);
extern protocol_context_t* vm_bootloader_protocol_get_context(void);
extern bool vm_bootloader_protocol_process_frame(vm_bootloader_context_internal_t* ctx);
extern void vm_bootloader_protocol_update_activity(void);
extern void vm_bootloader_protocol_reset_session(void);
extern vm_bootloader_state_t vm_bootloader_protocol_get_state(void);

#include <string.h>
#include <stdio.h>

// Build timestamp for version tracking
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

// Forward declarations for internal functions
static void vm_bootloader_context_init_defaults(vm_bootloader_context_internal_t* ctx);
static vm_bootloader_init_result_t vm_bootloader_init_subsystems(vm_bootloader_context_internal_t* ctx);
static void vm_bootloader_update_statistics(vm_bootloader_context_internal_t* ctx);
static bool vm_bootloader_process_frame(vm_bootloader_context_internal_t* ctx);
static void vm_bootloader_handle_timeout(vm_bootloader_context_internal_t* ctx);

// === LIFECYCLE MANAGEMENT IMPLEMENTATION ===

vm_bootloader_init_result_t vm_bootloader_init(vm_bootloader_context_t* ctx, const vm_bootloader_config_t* config)
{
    if (!ctx) {
        return VM_BOOTLOADER_INIT_ERROR_INVALID_CONFIG;
    }
    
    // Cast to internal context structure
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    
    // UART output test - compare string vs character
    uart_write_string("BOOTLOADER_INIT_TEST\r\n");
    uart_write_char('B'); // Bootloader init marker
    uart_write_char('I'); // Init marker
    uart_write_string("CHAR_TEST_COMPLETE\r\n");
    
    // Initialize context to safe defaults
    memset(internal_ctx, 0, sizeof(vm_bootloader_context_internal_t));
    vm_bootloader_context_init_defaults(internal_ctx);
    
    // Apply configuration
    if (config) {
        internal_ctx->session_timeout_ms = config->session_timeout_ms;
        internal_ctx->mode = config->initial_mode;
        internal_ctx->version_string = config->custom_version_info ? 
            config->custom_version_info : VM_BOOTLOADER_VERSION;
        internal_ctx->enable_debug_output = config->enable_debug_output;
        internal_ctx->enable_resource_tracking = config->enable_resource_tracking;
        internal_ctx->enable_emergency_recovery = config->enable_emergency_recovery;
        internal_ctx->frame_timeout_ms = config->frame_timeout_ms;
    } else {
        // Use default configuration
        vm_bootloader_config_t default_config;
        vm_bootloader_get_default_config(&default_config);
        internal_ctx->session_timeout_ms = default_config.session_timeout_ms;
        internal_ctx->mode = default_config.initial_mode;
        internal_ctx->version_string = VM_BOOTLOADER_VERSION;
        internal_ctx->enable_debug_output = default_config.enable_debug_output;
        internal_ctx->enable_resource_tracking = default_config.enable_resource_tracking;
        internal_ctx->enable_emergency_recovery = default_config.enable_emergency_recovery;
        internal_ctx->frame_timeout_ms = default_config.frame_timeout_ms;
    }
    
    internal_ctx->build_timestamp = BUILD_TIMESTAMP;
    internal_ctx->boot_time_ms = get_tick_ms();
    
    // Initialize all subsystems
    vm_bootloader_init_result_t result = vm_bootloader_init_subsystems(internal_ctx);
    if (result != VM_BOOTLOADER_INIT_SUCCESS) {
        return result;
    }
    
    // Mark as initialized
    internal_ctx->initialized = true;
    internal_ctx->current_state = VM_BOOTLOADER_STATE_IDLE;
    
    return VM_BOOTLOADER_INIT_SUCCESS;
}

vm_bootloader_run_result_t vm_bootloader_run_cycle(vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->initialized) {
        return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    // Check for emergency conditions
    if (internal_ctx->emergency_mode) {
        return VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN;
    }
    
    // Update execution cycle counter
    internal_ctx->execution_cycles++;
    
    // Check session timeout - but continue listening for Oracle
    if (vm_bootloader_is_session_timeout(ctx)) {
        // Reset session but keep listening for Oracle
        vm_bootloader_handle_timeout(internal_ctx);
        internal_ctx->session_active = true;  // Restart session
        internal_ctx->session_start_ms = get_tick_ms();
        // Don't return timeout - keep listening for Oracle
    }
    
    // Update activity timestamp
    internal_ctx->last_activity_ms = get_tick_ms();
    
    // Process incoming frames using protocol engine
    bool frame_processed = vm_bootloader_process_frame(internal_ctx);
    if (frame_processed) {
        internal_ctx->total_frames_received++;
        vm_bootloader_update_statistics(internal_ctx);
    }
    
    // Update protocol engine activity
    if (internal_ctx->protocol_ctx) {
        vm_bootloader_protocol_update_activity();
    }
    
    // Check if session is complete using protocol state
    if (internal_ctx->session_active) {
        if (internal_ctx->current_state == VM_BOOTLOADER_STATE_COMPLETE) {
            internal_ctx->session_active = false;
            internal_ctx->successful_operations++;
            return VM_BOOTLOADER_RUN_COMPLETE;
        }
    }
    
    return VM_BOOTLOADER_RUN_CONTINUE;
}

vm_bootloader_run_result_t vm_bootloader_main_loop(vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->initialized) {
        return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
    }
    
    // Start session
    internal_ctx->session_active = true;
    internal_ctx->session_start_ms = get_tick_ms();
    internal_ctx->current_state = VM_BOOTLOADER_STATE_IDLE;
    
    // Debug output for Oracle integration
    if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG || internal_ctx->mode == VM_BOOTLOADER_MODE_LISTEN_ONLY) {
        uart_write_string("CockpitVM Bootloader entering main loop\r\n");
        uart_write_string("Session timeout: ");
        // Simple integer to string for timeout display
        uint32_t timeout_sec = internal_ctx->session_timeout_ms / 1000;
        char timeout_str[16];
        snprintf(timeout_str, sizeof(timeout_str), "%lu", timeout_sec);
        uart_write_string(timeout_str);
        uart_write_string(" seconds\r\n");
    }
    
    vm_bootloader_run_result_t result;
    
    // Main processing loop
    while (true) {
        result = vm_bootloader_run_cycle(ctx);
        
        switch (result) {
            case VM_BOOTLOADER_RUN_CONTINUE:
                // Continue normal operation
                break;
                
            case VM_BOOTLOADER_RUN_COMPLETE:
                // Session completed successfully
                if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("CockpitVM Bootloader session complete - success\r\n");
                }
                return VM_BOOTLOADER_RUN_COMPLETE;
                
            case VM_BOOTLOADER_RUN_TIMEOUT:
                // Session timeout occurred  
                if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("CockpitVM Bootloader session timeout\r\n");
                }
                return VM_BOOTLOADER_RUN_TIMEOUT;
                
            case VM_BOOTLOADER_RUN_ERROR_RECOVERABLE:
                // Recoverable error - continue with caution
                internal_ctx->total_errors++;
                if (internal_ctx->total_errors > 10) {
                    // Too many errors - escalate to critical
                    return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
                }
                break;
                
            case VM_BOOTLOADER_RUN_ERROR_CRITICAL:
                // Critical error - emergency shutdown
                if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("CockpitVM Bootloader critical error - emergency shutdown\r\n");
                }
                return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
                
            case VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN:
                // Emergency shutdown initiated
                if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
                    uart_write_string("CockpitVM Bootloader emergency shutdown\r\n");
                }
                vm_bootloader_emergency_shutdown(ctx);
                return VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN;
                
            default:
                // Unknown result - treat as critical error
                return VM_BOOTLOADER_RUN_ERROR_CRITICAL;
        }
        
        // Small delay to prevent busy waiting
        delay_ms(10);
    }
}

void vm_bootloader_emergency_shutdown(vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    
    // Mark emergency mode
    internal_ctx->emergency_mode = true;
    
    // Emergency LED pattern - rapid blink
    for (int i = 0; i < 5; i++) {
        gpio_pin_write(13, true);  // PC6 = pin 13
        delay_ms(100);
        gpio_pin_write(13, false);
        delay_ms(100);
    }
    
    // Clean up critical resources first
    // TODO: Resource manager integration in Chunk 3
    
    // Put hardware in safe state
    uart_write_string("EMERGENCY: CockpitVM Bootloader entering safe state\r\n");
    
    // Reset UART to known state
    uart_begin(115200);
    
    // Set recovery state
    internal_ctx->current_state = VM_BOOTLOADER_STATE_RECOVERY_ABORT;
}

void vm_bootloader_cleanup(vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->initialized) {
        return;
    }
    
    // Clean session statistics
    if (internal_ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
        uart_write_string("CockpitVM Bootloader cleanup - statistics:\r\n");
        // Output basic statistics
        char stats_msg[64];
        snprintf(stats_msg, sizeof(stats_msg), "Cycles: %lu, Frames: %lu, Errors: %lu\r\n",
                internal_ctx->execution_cycles, internal_ctx->total_frames_received, internal_ctx->total_errors);
        uart_write_string(stats_msg);
    }
    
    // TODO: Resource manager cleanup in Chunk 3
    
    // Protocol engine cleanup
    if (internal_ctx->protocol_ctx) {
        vm_bootloader_protocol_reset_session();
    }
    
    // Mark as not initialized
    internal_ctx->initialized = false;
    internal_ctx->session_active = false;
    internal_ctx->emergency_mode = false;
}

// === CONTEXT QUERY IMPLEMENTATION ===

bool vm_bootloader_is_initialized(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return false;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    return internal_ctx->initialized;
}

bool vm_bootloader_is_ready(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return false;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->initialized || internal_ctx->emergency_mode) {
        return false;
    }
    
    return internal_ctx->current_state == VM_BOOTLOADER_STATE_IDLE ||
           internal_ctx->current_state == VM_BOOTLOADER_STATE_READY;
}

bool vm_bootloader_is_session_timeout(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return true;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->session_active) {
        return false;
    }
    
    uint32_t current_time = get_tick_ms();
    uint32_t elapsed = current_time - internal_ctx->session_start_ms;
    
    return elapsed > internal_ctx->session_timeout_ms;
}

vm_bootloader_state_t vm_bootloader_get_current_state(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return VM_BOOTLOADER_STATE_RECOVERY_ABORT;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    return internal_ctx->current_state;
}

uint32_t vm_bootloader_get_uptime_ms(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return 0;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    
    uint32_t current_time = get_tick_ms();
    return current_time - internal_ctx->boot_time_ms;
}

uint32_t vm_bootloader_get_session_elapsed_ms(const vm_bootloader_context_t* ctx)
{
    if (!ctx) {
        return 0;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    
    if (!internal_ctx->session_active) {
        return 0;
    }
    
    uint32_t current_time = get_tick_ms();
    return current_time - internal_ctx->session_start_ms;
}

// === CONFIGURATION IMPLEMENTATION ===

void vm_bootloader_set_mode(vm_bootloader_context_t* ctx, vm_bootloader_mode_t mode)
{
    if (!ctx) {
        return;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    internal_ctx->mode = mode;
}

void vm_bootloader_set_debug_mode(vm_bootloader_context_t* ctx, bool enabled)
{
    if (!ctx) {
        return;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    internal_ctx->mode = enabled ? VM_BOOTLOADER_MODE_DEBUG : VM_BOOTLOADER_MODE_NORMAL;
}

void vm_bootloader_set_session_timeout(vm_bootloader_context_t* ctx, uint32_t timeout_ms)
{
    if (!ctx) {
        return;
    }
    
    vm_bootloader_context_internal_t* internal_ctx = (vm_bootloader_context_internal_t*)ctx;
    internal_ctx->session_timeout_ms = timeout_ms;
}

// === STATISTICS IMPLEMENTATION ===

void vm_bootloader_get_statistics(const vm_bootloader_context_t* ctx, vm_bootloader_statistics_t* stats)
{
    if (!ctx || !stats) {
        return;
    }
    
    const vm_bootloader_context_internal_t* internal_ctx = (const vm_bootloader_context_internal_t*)ctx;
    
    stats->uptime_ms = vm_bootloader_get_uptime_ms(ctx);
    stats->execution_cycles = internal_ctx->execution_cycles;
    stats->frames_received = internal_ctx->total_frames_received;
    stats->frames_sent = internal_ctx->total_frames_sent;
    stats->total_errors = internal_ctx->total_errors;
    stats->successful_operations = internal_ctx->successful_operations;
    stats->current_state = internal_ctx->current_state;
    stats->current_mode = internal_ctx->mode;
}

// === CONFIGURATION HELPERS ===

void vm_bootloader_get_default_config(vm_bootloader_config_t* config)
{
    if (!config) {
        return;
    }
    
    config->session_timeout_ms = 30000;          // 30 seconds
    config->frame_timeout_ms = 500;              // 500ms
    config->initial_mode = VM_BOOTLOADER_MODE_NORMAL;
    config->enable_debug_output = false;
    config->enable_resource_tracking = true;
    config->enable_emergency_recovery = true;
    config->custom_version_info = NULL;
}

void vm_bootloader_get_oracle_config(vm_bootloader_config_t* config)
{
    if (!config) {
        return;
    }
    
    // Oracle testing configuration
    config->session_timeout_ms = 30000;          // 30 seconds for Oracle testing
    config->frame_timeout_ms = 500;              // 500ms frame timeout
    config->initial_mode = VM_BOOTLOADER_MODE_LISTEN_ONLY;
    config->enable_debug_output = true;          // Enable debug for Oracle
    config->enable_resource_tracking = true;
    config->enable_emergency_recovery = true;
    config->custom_version_info = "4.5.2-Oracle";
}

// === STATE UTILITIES ===

const char* vm_bootloader_get_state_name(vm_bootloader_state_t state)
{
    // Map VM bootloader states to bootloader_states for now
    // This will be unified in Chunk 3
    return bootloader_get_state_name((bootloader_state_t)state);
}

bool vm_bootloader_is_error_state(vm_bootloader_state_t state)
{
    return bootloader_is_error_state((bootloader_state_t)state);
}

bool vm_bootloader_state_allows_retry(vm_bootloader_state_t state)
{
    return bootloader_state_allows_retry((bootloader_state_t)state);
}

// === INTERNAL IMPLEMENTATION ===

static void vm_bootloader_context_init_defaults(vm_bootloader_context_internal_t* ctx)
{
    ctx->initialized = false;
    ctx->emergency_mode = false;
    ctx->session_active = false;
    ctx->current_state = VM_BOOTLOADER_STATE_INIT;
    ctx->mode = VM_BOOTLOADER_MODE_NORMAL;
    ctx->session_timeout_ms = 30000;
    ctx->frame_timeout_ms = 500;
    ctx->execution_cycles = 0;
    ctx->total_frames_received = 0;
    ctx->total_frames_sent = 0;
    ctx->total_errors = 0;
    ctx->successful_operations = 0;
    
    // Initialize timestamps
    ctx->boot_time_ms = 0;
    ctx->session_start_ms = 0;
    ctx->last_activity_ms = 0;
    
    // Configuration flags
    ctx->enable_debug_output = false;
    ctx->enable_resource_tracking = true;
    ctx->enable_emergency_recovery = true;
    
    // Version and build info
    ctx->version_string = VM_BOOTLOADER_VERSION;
    ctx->build_timestamp = BUILD_TIMESTAMP;
}

static vm_bootloader_init_result_t vm_bootloader_init_subsystems(vm_bootloader_context_internal_t* ctx)
{
    // Initialize host interface (UART, GPIO, timing)
    host_interface_init();
    
    // Configure UART for protocol communication
    uart_begin(115200);
    
    // Configure status LED
    gpio_pin_config(13, GPIO_OUTPUT);  // PC6 = pin 13
    
    // Initialize protocol engine
    vm_bootloader_protocol_engine_init();
    ctx->protocol_ctx = (vm_bootloader_protocol_context_t*)vm_bootloader_protocol_get_context();
    if (!ctx->protocol_ctx) {
        return VM_BOOTLOADER_INIT_ERROR_PROTOCOL_FAILED;
    }
    
    // TODO: Initialize resource manager in Chunk 3
    // For now, set resource managers to NULL
    ctx->resource_mgr = NULL;
    ctx->error_mgr = NULL;
    ctx->timeout_mgr = NULL;
    
    return VM_BOOTLOADER_INIT_SUCCESS;
}

static void vm_bootloader_update_statistics(vm_bootloader_context_internal_t* ctx)
{
    ctx->last_activity_ms = get_tick_ms();
    
    // Update session activity
    if (!ctx->session_active && ctx->current_state != VM_BOOTLOADER_STATE_IDLE) {
        ctx->session_active = true;
        ctx->session_start_ms = ctx->last_activity_ms;
    }
}

static bool vm_bootloader_process_frame(vm_bootloader_context_internal_t* ctx)
{
    // Use integrated protocol engine for frame processing
    return vm_bootloader_protocol_process_frame(ctx);
}

static void vm_bootloader_handle_timeout(vm_bootloader_context_internal_t* ctx)
{
    ctx->session_active = false;
    ctx->current_state = VM_BOOTLOADER_STATE_IDLE;
    
    if (ctx->mode == VM_BOOTLOADER_MODE_DEBUG) {
        uart_write_string("CockpitVM Bootloader session timeout handled\r\n");
    }
}