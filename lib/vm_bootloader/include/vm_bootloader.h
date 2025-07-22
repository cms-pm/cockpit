/*
 * CockpitVM Unified Bootloader
 * 
 * Single API for bootloader context management, protocol engine, and state machine.
 * Consolidates bootloader_framework, bootloader_protocol, and bootloader_states
 * into an embedded-first architecture optimized for STM32G431CB.
 */

#ifndef VM_BOOTLOADER_H
#define VM_BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version and build information
#define VM_BOOTLOADER_VERSION "4.5.2"
#define VM_BOOTLOADER_PROTOCOL_VERSION "4.5.2"

// Forward declarations and opaque context structure
// Note: Actual size determined by internal implementation
typedef struct {
    // Opaque context data - do not access directly
    // Use vm_bootloader_* API functions for all operations
    uint8_t _internal_data[256];  // Sufficient space for internal context
} vm_bootloader_context_t;

// Bootloader modes
typedef enum {
    VM_BOOTLOADER_MODE_NORMAL = 0,        // Production mode
    VM_BOOTLOADER_MODE_DEBUG,             // Debug output enabled
    VM_BOOTLOADER_MODE_LISTEN_ONLY,       // Oracle testing mode
    VM_BOOTLOADER_MODE_RECOVERY           // Recovery mode
} vm_bootloader_mode_t;

// Bootloader initialization results
typedef enum {
    VM_BOOTLOADER_INIT_SUCCESS = 0,
    VM_BOOTLOADER_INIT_ERROR_INVALID_CONFIG,
    VM_BOOTLOADER_INIT_ERROR_PROTOCOL_FAILED,
    VM_BOOTLOADER_INIT_ERROR_HARDWARE_FAILED,
    VM_BOOTLOADER_INIT_ERROR_RESOURCE_EXHAUSTION
} vm_bootloader_init_result_t;

// Bootloader runtime results
typedef enum {
    VM_BOOTLOADER_RUN_CONTINUE = 0,       // Continue normal operation
    VM_BOOTLOADER_RUN_COMPLETE,           // Session completed successfully
    VM_BOOTLOADER_RUN_TIMEOUT,            // Session timeout occurred
    VM_BOOTLOADER_RUN_ERROR_RECOVERABLE,  // Recoverable error
    VM_BOOTLOADER_RUN_ERROR_CRITICAL,     // Critical error
    VM_BOOTLOADER_RUN_EMERGENCY_SHUTDOWN  // Emergency shutdown
} vm_bootloader_run_result_t;

// Bootloader states (from bootloader_states.h)
typedef enum {
    VM_BOOTLOADER_STATE_INIT = 0,
    VM_BOOTLOADER_STATE_IDLE,
    VM_BOOTLOADER_STATE_HANDSHAKE,
    VM_BOOTLOADER_STATE_READY,
    VM_BOOTLOADER_STATE_RECEIVE_DATA,
    VM_BOOTLOADER_STATE_VERIFY,
    VM_BOOTLOADER_STATE_PROGRAM,
    VM_BOOTLOADER_STATE_COMPLETE,
    
    // Error states
    VM_BOOTLOADER_STATE_ERROR_COMMUNICATION,
    VM_BOOTLOADER_STATE_ERROR_FLASH_OPERATION,
    VM_BOOTLOADER_STATE_ERROR_DATA_CORRUPTION,
    VM_BOOTLOADER_STATE_ERROR_RESOURCE_EXHAUSTION,
    
    // Recovery states
    VM_BOOTLOADER_STATE_RECOVERY_RETRY,
    VM_BOOTLOADER_STATE_RECOVERY_ABORT
} vm_bootloader_state_t;

// Configuration structure
typedef struct {
    uint32_t session_timeout_ms;          // Session timeout (default: 30000)
    uint32_t frame_timeout_ms;            // Frame timeout (default: 500)
    vm_bootloader_mode_t initial_mode;    // Initial operating mode
    bool enable_debug_output;             // Debug UART output
    bool enable_resource_tracking;       // Resource usage tracking
    bool enable_emergency_recovery;      // Emergency recovery features
    const char* custom_version_info;     // Custom version string
} vm_bootloader_config_t;

// Statistics structure
typedef struct {
    uint32_t uptime_ms;                   // Total uptime since boot
    uint32_t execution_cycles;            // Main loop execution cycles
    uint32_t frames_received;             // Total frames received
    uint32_t frames_sent;                 // Total frames sent
    uint32_t total_errors;                // Total error count
    uint32_t successful_operations;       // Successful operations count
    vm_bootloader_state_t current_state;  // Current state
    vm_bootloader_mode_t current_mode;    // Current mode
} vm_bootloader_statistics_t;

// === LIFECYCLE MANAGEMENT API ===

// Initialize bootloader context
vm_bootloader_init_result_t vm_bootloader_init(vm_bootloader_context_t* ctx, 
                                               const vm_bootloader_config_t* config);

// Run single bootloader cycle
vm_bootloader_run_result_t vm_bootloader_run_cycle(vm_bootloader_context_t* ctx);

// Main bootloader loop (Oracle integration point)
vm_bootloader_run_result_t vm_bootloader_main_loop(vm_bootloader_context_t* ctx);

// Emergency shutdown
void vm_bootloader_emergency_shutdown(vm_bootloader_context_t* ctx);

// Cleanup bootloader context
void vm_bootloader_cleanup(vm_bootloader_context_t* ctx);

// === CONTEXT QUERY API ===

// Check if bootloader is initialized
bool vm_bootloader_is_initialized(const vm_bootloader_context_t* ctx);

// Check if bootloader is ready for operations
bool vm_bootloader_is_ready(const vm_bootloader_context_t* ctx);

// Check for session timeout
bool vm_bootloader_is_session_timeout(const vm_bootloader_context_t* ctx);

// Get current state
vm_bootloader_state_t vm_bootloader_get_current_state(const vm_bootloader_context_t* ctx);

// Get uptime in milliseconds
uint32_t vm_bootloader_get_uptime_ms(const vm_bootloader_context_t* ctx);

// Get session elapsed time
uint32_t vm_bootloader_get_session_elapsed_ms(const vm_bootloader_context_t* ctx);

// === CONFIGURATION API ===

// Set bootloader mode
void vm_bootloader_set_mode(vm_bootloader_context_t* ctx, vm_bootloader_mode_t mode);

// Enable/disable debug mode
void vm_bootloader_set_debug_mode(vm_bootloader_context_t* ctx, bool enabled);

// Set session timeout
void vm_bootloader_set_session_timeout(vm_bootloader_context_t* ctx, uint32_t timeout_ms);

// === STATISTICS API ===

// Get runtime statistics
void vm_bootloader_get_statistics(const vm_bootloader_context_t* ctx, 
                                  vm_bootloader_statistics_t* stats);

// === CONFIGURATION HELPERS ===

// Get default configuration
void vm_bootloader_get_default_config(vm_bootloader_config_t* config);

// Get Oracle testing configuration
void vm_bootloader_get_oracle_config(vm_bootloader_config_t* config);

// === STATE UTILITIES ===

// Get state name as string
const char* vm_bootloader_get_state_name(vm_bootloader_state_t state);

// Check if state is error state
bool vm_bootloader_is_error_state(vm_bootloader_state_t state);

// Check if state allows retry
bool vm_bootloader_state_allows_retry(vm_bootloader_state_t state);

#ifdef __cplusplus
}
#endif

#endif // VM_BOOTLOADER_H