/*
 * CockpitVM Bootloader Framework - Context Management
 * 
 * Provides high-level bootloader lifecycle management that integrates
 * the protocol library with complete system management including resource
 * cleanup, error handling, and emergency shutdown capabilities.
 * 
 * This bridges the gap between the protocol library (messaging) and a 
 * complete production bootloader system (lifecycle + reliability).
 */

#ifndef BOOTLOADER_CONTEXT_H
#define BOOTLOADER_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

// Dependencies on canonical protocol implementation
#include "bootloader_protocol.h"
#include "bootloader_states.h"

#ifdef __cplusplus
extern "C" {
#endif

// Version information
#define BOOTLOADER_FRAMEWORK_VERSION "4.5.2"
#define BOOTLOADER_FRAMEWORK_VERSION_MAJOR 4
#define BOOTLOADER_FRAMEWORK_VERSION_MINOR 5
#define BOOTLOADER_FRAMEWORK_VERSION_PATCH 2

// Forward declarations for integrated subsystems
typedef struct resource_manager resource_manager_t;
typedef struct error_manager error_manager_t;
typedef struct timeout_manager timeout_manager_t;

// Bootloader initialization results
typedef enum {
    BOOTLOADER_INIT_SUCCESS = 0,
    BOOTLOADER_INIT_ERROR_PROTOCOL_FAILED,
    BOOTLOADER_INIT_ERROR_RESOURCE_MANAGER_FAILED,
    BOOTLOADER_INIT_ERROR_UART_FAILED,
    BOOTLOADER_INIT_ERROR_FLASH_FAILED,
    BOOTLOADER_INIT_ERROR_EMERGENCY_MODE
} bootloader_init_result_t;

// Bootloader run cycle results
typedef enum {
    BOOTLOADER_RUN_CONTINUE = 0,      // Continue normal operation
    BOOTLOADER_RUN_COMPLETE,          // Session complete, success
    BOOTLOADER_RUN_TIMEOUT,           // Session timeout, clean exit
    BOOTLOADER_RUN_ERROR_RECOVERABLE, // Error occurred, but can continue
    BOOTLOADER_RUN_ERROR_CRITICAL,    // Critical error, emergency shutdown needed
    BOOTLOADER_RUN_EMERGENCY_SHUTDOWN // Emergency shutdown initiated
} bootloader_run_result_t;

// Bootloader operational modes
typedef enum {
    BOOTLOADER_MODE_NORMAL = 0,       // Standard operation
    BOOTLOADER_MODE_DEBUG,            // Enhanced debug output
    BOOTLOADER_MODE_EMERGENCY,        // Emergency recovery mode
    BOOTLOADER_MODE_LISTEN_ONLY       // Listen mode for Oracle testing
} bootloader_mode_t;

// Complete bootloader context - integrates all subsystems
typedef struct {
    // Core protocol context (from canonical implementation)
    protocol_context_t* protocol_ctx;
    
    // System management subsystems
    resource_manager_t* resource_mgr;
    error_manager_t* error_mgr;
    timeout_manager_t* timeout_mgr;
    
    // Bootloader state and mode
    bootloader_state_t current_state;
    bootloader_mode_t mode;
    
    // Lifecycle tracking
    bool initialized;
    bool emergency_mode;
    uint32_t boot_time_ms;
    uint32_t execution_cycles;
    uint32_t last_activity_ms;
    
    // Session management
    uint32_t session_start_ms;
    uint32_t session_timeout_ms;      // Default: 30 seconds
    bool session_active;
    
    // Statistics and diagnostics
    uint32_t total_frames_received;
    uint32_t total_frames_sent;
    uint32_t total_errors;
    uint32_t successful_operations;
    
    // Version and build info
    const char* version_string;
    const char* build_timestamp;
} bootloader_context_t;

// Configuration structure for bootloader initialization
typedef struct {
    uint32_t session_timeout_ms;      // Default: 30000 (30 seconds)
    uint32_t frame_timeout_ms;        // Default: 500 (500ms)
    bootloader_mode_t initial_mode;   // Default: BOOTLOADER_MODE_NORMAL
    bool enable_debug_output;         // Default: false
    bool enable_resource_tracking;    // Default: true
    bool enable_emergency_recovery;   // Default: true
    const char* custom_version_info; // Optional custom version string
} bootloader_config_t;

// === LIFECYCLE MANAGEMENT API ===

/**
 * Initialize the complete bootloader framework
 * This coordinates initialization of all subsystems:
 * - Protocol context
 * - Resource manager  
 * - Error manager
 * - Timeout manager
 * - UART and flash subsystems
 */
bootloader_init_result_t bootloader_init(bootloader_context_t* ctx, const bootloader_config_t* config);

/**
 * Run a single bootloader processing cycle
 * This handles:
 * - Frame reception and processing
 * - Timeout management
 * - Resource cleanup
 * - Error recovery
 * - State transitions
 */
bootloader_run_result_t bootloader_run_cycle(bootloader_context_t* ctx);

/**
 * Run the complete bootloader main loop
 * Continuously calls bootloader_run_cycle() until completion or error
 * Handles session timeouts and recovery automatically
 */
bootloader_run_result_t bootloader_main_loop(bootloader_context_t* ctx);

/**
 * Emergency shutdown with complete cleanup
 * - Cleans up all registered resources
 * - Flushes any pending operations
 * - Puts hardware into safe state
 * - Logs emergency context for debugging
 */
void bootloader_emergency_shutdown(bootloader_context_t* ctx);

/**
 * Normal bootloader cleanup
 * - Clean session termination
 * - Resource cleanup
 * - Statistics logging
 */
void bootloader_cleanup(bootloader_context_t* ctx);

// === CONTEXT QUERY API ===

/**
 * Check if bootloader is properly initialized
 */
bool bootloader_is_initialized(const bootloader_context_t* ctx);

/**
 * Check if bootloader is ready to accept commands
 */
bool bootloader_is_ready(const bootloader_context_t* ctx);

/**
 * Check if session timeout has occurred
 */
bool bootloader_is_session_timeout(const bootloader_context_t* ctx);

/**
 * Get current bootloader state
 */
bootloader_state_t bootloader_get_current_state(const bootloader_context_t* ctx);

/**
 * Get bootloader uptime in milliseconds
 */
uint32_t bootloader_get_uptime_ms(const bootloader_context_t* ctx);

/**
 * Get session elapsed time in milliseconds
 */
uint32_t bootloader_get_session_elapsed_ms(const bootloader_context_t* ctx);

// === CONFIGURATION API ===

/**
 * Set bootloader operational mode
 */
void bootloader_set_mode(bootloader_context_t* ctx, bootloader_mode_t mode);

/**
 * Enable/disable debug mode
 */
void bootloader_set_debug_mode(bootloader_context_t* ctx, bool enabled);

/**
 * Set custom session timeout
 */
void bootloader_set_session_timeout(bootloader_context_t* ctx, uint32_t timeout_ms);

// === STATISTICS API ===

/**
 * Get bootloader statistics
 */
typedef struct {
    uint32_t uptime_ms;
    uint32_t execution_cycles;
    uint32_t frames_received;
    uint32_t frames_sent;
    uint32_t total_errors;
    uint32_t successful_operations;
    bootloader_state_t current_state;
    bootloader_mode_t current_mode;
} bootloader_statistics_t;

void bootloader_get_statistics(const bootloader_context_t* ctx, bootloader_statistics_t* stats);

// === DEFAULT CONFIGURATION ===

/**
 * Get default bootloader configuration
 */
void bootloader_get_default_config(bootloader_config_t* config);

/**
 * Create a standard Oracle testing configuration
 */
void bootloader_get_oracle_config(bootloader_config_t* config);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_CONTEXT_H