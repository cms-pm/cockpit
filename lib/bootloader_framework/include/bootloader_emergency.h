/*
 * ComponentVM Bootloader Framework - Emergency Management
 * 
 * Provides emergency shutdown and recovery capabilities for critical
 * bootloader failures. Ensures system can be put into safe state
 * even during catastrophic errors.
 * 
 * Critical for production embedded systems where bootloader failures
 * could result in unrecoverable hardware states.
 */

#ifndef BOOTLOADER_EMERGENCY_H
#define BOOTLOADER_EMERGENCY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Emergency conditions that trigger emergency shutdown
typedef enum {
    EMERGENCY_CONDITION_NONE = 0,
    EMERGENCY_CONDITION_RESOURCE_EXHAUSTION,   // Out of memory/resources
    EMERGENCY_CONDITION_HARDWARE_FAULT,        // Hardware not responding
    EMERGENCY_CONDITION_COMMUNICATION_FAILURE, // UART/transport failure
    EMERGENCY_CONDITION_FLASH_CORRUPTION,     // Flash operations failing
    EMERGENCY_CONDITION_TIMEOUT_EXCEEDED,     // Critical timeout exceeded
    EMERGENCY_CONDITION_PROTOCOL_VIOLATION,   // Protocol state corruption
    EMERGENCY_CONDITION_WATCHDOG_TRIGGER,     // Watchdog about to reset
    EMERGENCY_CONDITION_USER_REQUESTED        // Emergency shutdown requested
} emergency_condition_t;

// Emergency shutdown phases
typedef enum {
    EMERGENCY_PHASE_DETECT = 0,               // Emergency condition detected
    EMERGENCY_PHASE_SIGNAL,                   // Signal emergency to system
    EMERGENCY_PHASE_CRITICAL_CLEANUP,         // Clean up critical resources
    EMERGENCY_PHASE_HARDWARE_SAFE_STATE,      // Put hardware in safe state
    EMERGENCY_PHASE_DIAGNOSTICS,              // Log diagnostic information
    EMERGENCY_PHASE_FINAL_SHUTDOWN            // Final system shutdown
} emergency_phase_t;

// Emergency recovery actions
typedef enum {
    EMERGENCY_ACTION_NONE = 0,
    EMERGENCY_ACTION_RESTART_SESSION,         // Restart bootloader session
    EMERGENCY_ACTION_RESET_PROTOCOL,          // Reset protocol state machine
    EMERGENCY_ACTION_REINITIALIZE_UART,       // Reinitialize UART
    EMERGENCY_ACTION_FLUSH_BUFFERS,           // Flush all buffers
    EMERGENCY_ACTION_HARDWARE_RESET,          // Full hardware reset
    EMERGENCY_ACTION_SAFE_MODE               // Enter safe mode operation
} emergency_action_t;

// Emergency context for diagnostics
typedef struct {
    emergency_condition_t condition;          // What triggered emergency
    emergency_phase_t current_phase;          // Current emergency phase
    uint32_t timestamp;                       // When emergency was triggered
    
    // Diagnostic context
    const char* trigger_file;                 // __FILE__ where emergency triggered
    uint32_t trigger_line;                    // __LINE__ where emergency triggered
    const char* trigger_function;             // Function where emergency triggered
    const char* diagnostic_message;           // Human-readable description
    
    // System state at time of emergency
    uint32_t bootloader_state;                // Bootloader state
    uint32_t protocol_state;                  // Protocol state
    uint32_t system_uptime_ms;               // System uptime
    uint32_t session_elapsed_ms;             // Session elapsed time
    uint32_t active_resources;               // Number of active resources
    
    // Recovery information
    emergency_action_t recovery_action;       // Recommended recovery action
    bool recovery_attempted;                  // Has recovery been attempted
    bool recovery_successful;                 // Was recovery successful
    uint32_t recovery_attempts;              // Number of recovery attempts
} emergency_context_t;

// Emergency manager state
typedef struct {
    bool initialized;
    bool emergency_active;
    emergency_context_t current_emergency;
    
    // Emergency history (for debugging)
    emergency_context_t emergency_history[4]; // Last 4 emergencies
    uint8_t history_count;
    uint8_t history_index;
    
    // Statistics
    uint32_t total_emergencies;
    uint32_t successful_recoveries;
    uint32_t failed_recoveries;
    
    // Configuration
    bool enable_auto_recovery;
    uint32_t max_recovery_attempts;
    uint32_t recovery_delay_ms;
} emergency_manager_t;

// Emergency callback function signatures
typedef void (*emergency_callback_t)(const emergency_context_t* ctx);
typedef bool (*recovery_callback_t)(emergency_condition_t condition, uint32_t attempt);

// === EMERGENCY MANAGER LIFECYCLE ===

/**
 * Initialize emergency management system
 */
void emergency_manager_init(emergency_manager_t* mgr);

/**
 * Cleanup emergency management system
 */
void emergency_manager_cleanup(emergency_manager_t* mgr);

// === EMERGENCY TRIGGERING ===

/**
 * Trigger emergency shutdown with diagnostic context
 */
void emergency_trigger(
    emergency_manager_t* mgr,
    emergency_condition_t condition,
    const char* diagnostic_message,
    const char* file,
    uint32_t line,
    const char* function
);

/**
 * Trigger emergency with recovery action suggestion
 */
void emergency_trigger_with_recovery(
    emergency_manager_t* mgr,
    emergency_condition_t condition,
    emergency_action_t recovery_action,
    const char* diagnostic_message,
    const char* file,
    uint32_t line,
    const char* function
);

// === EMERGENCY RECOVERY ===

/**
 * Attempt automatic recovery from emergency condition
 */
bool emergency_attempt_recovery(
    emergency_manager_t* mgr,
    recovery_callback_t recovery_callback
);

/**
 * Check if system is in emergency state
 */
bool emergency_is_active(const emergency_manager_t* mgr);

/**
 * Clear emergency state after successful recovery
 */
void emergency_clear(emergency_manager_t* mgr);

/**
 * Reset emergency manager (clear history, reset state)
 */
void emergency_reset(emergency_manager_t* mgr);

// === EMERGENCY ACTIONS ===

/**
 * Execute emergency shutdown sequence
 * This performs the complete emergency shutdown process:
 * 1. Signal emergency condition
 * 2. Clean up critical resources
 * 3. Put hardware in safe state  
 * 4. Log diagnostics
 * 5. Perform final shutdown
 */
void emergency_execute_shutdown(
    emergency_manager_t* mgr,
    emergency_callback_t emergency_callback
);

/**
 * Put system hardware into safe state
 * - Disable interrupts
 * - Reset UART to safe state
 * - Turn off LEDs/indicators
 * - Release flash resources
 */
void emergency_hardware_safe_state(void);

/**
 * Emergency diagnostic logging
 * Log critical system state for post-emergency analysis
 */
void emergency_log_diagnostics(const emergency_context_t* ctx);

// === EMERGENCY QUERIES ===

/**
 * Get current emergency context
 */
const emergency_context_t* emergency_get_current_context(const emergency_manager_t* mgr);

/**
 * Get emergency history
 */
void emergency_get_history(
    const emergency_manager_t* mgr,
    emergency_context_t* history_buffer,
    uint8_t buffer_size,
    uint8_t* actual_count
);

/**
 * Get emergency statistics
 */
typedef struct {
    uint32_t total_emergencies;
    uint32_t successful_recoveries;
    uint32_t failed_recoveries;
    bool emergency_active;
    emergency_condition_t last_condition;
    uint32_t time_since_last_emergency;
} emergency_stats_t;

void emergency_get_stats(const emergency_manager_t* mgr, emergency_stats_t* stats);

// === EMERGENCY CONFIGURATION ===

/**
 * Configure emergency recovery behavior
 */
void emergency_configure(
    emergency_manager_t* mgr,
    bool enable_auto_recovery,
    uint32_t max_recovery_attempts,
    uint32_t recovery_delay_ms
);

/**
 * Check if emergency condition warrants automatic recovery
 */
bool emergency_can_attempt_recovery(
    const emergency_manager_t* mgr,
    emergency_condition_t condition
);

// === CONVENIENCE MACROS ===

// Macro for triggering emergency with automatic file/line/function info
#define EMERGENCY_TRIGGER(mgr, condition, message) \
    emergency_trigger(mgr, condition, message, __FILE__, __LINE__, __func__)

#define EMERGENCY_TRIGGER_WITH_RECOVERY(mgr, condition, recovery_action, message) \
    emergency_trigger_with_recovery(mgr, condition, recovery_action, message, __FILE__, __LINE__, __func__)

// Emergency condition checks
#define EMERGENCY_CHECK_RESOURCE_EXHAUSTION(mgr, condition, message) \
    do { if (condition) { EMERGENCY_TRIGGER(mgr, EMERGENCY_CONDITION_RESOURCE_EXHAUSTION, message); } } while(0)

#define EMERGENCY_CHECK_HARDWARE_FAULT(mgr, condition, message) \
    do { if (condition) { EMERGENCY_TRIGGER(mgr, EMERGENCY_CONDITION_HARDWARE_FAULT, message); } } while(0)

#define EMERGENCY_CHECK_TIMEOUT(mgr, condition, message) \
    do { if (condition) { EMERGENCY_TRIGGER(mgr, EMERGENCY_CONDITION_TIMEOUT_EXCEEDED, message); } } while(0)

// === STANDARD RECOVERY FUNCTIONS ===

/**
 * Standard recovery functions for common emergency conditions
 */
bool emergency_recovery_restart_session(emergency_condition_t condition, uint32_t attempt);
bool emergency_recovery_reset_protocol(emergency_condition_t condition, uint32_t attempt);
bool emergency_recovery_reinit_uart(emergency_condition_t condition, uint32_t attempt);
bool emergency_recovery_hardware_reset(emergency_condition_t condition, uint32_t attempt);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_EMERGENCY_H