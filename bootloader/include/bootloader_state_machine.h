#ifndef BOOTLOADER_STATE_MACHINE_H
#define BOOTLOADER_STATE_MACHINE_H

#include "bootloader_errors.h"
#include "timeout_manager.h"
#include "resource_manager.h"
#include "transport_interface.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bootloader_state_t current_state;
    bootloader_state_t next_state;
    bootloader_state_t previous_state;
    
    uint32_t state_entry_time;
    uint32_t state_execution_count;
    uint32_t state_transition_count;
    
    timeout_context_t state_timeout;
    timeout_context_t operation_timeout;
    
    transport_context_t transport;
    
    error_manager_t* error_manager;
    resource_manager_t* resource_manager;
    timeout_manager_t* timeout_manager;
    
    bool state_change_pending;
    bool emergency_mode;
    bool debug_mode;
    
    uint8_t state_retry_count;
    uint8_t max_state_retries;
    
    void* user_context;
} bootloader_state_machine_t;

typedef enum {
    STATE_TRANSITION_OK = 0,
    STATE_TRANSITION_ERROR_INVALID_STATE,
    STATE_TRANSITION_ERROR_RESOURCE_BUSY,
    STATE_TRANSITION_ERROR_TIMEOUT,
    STATE_TRANSITION_ERROR_CRITICAL_FAILURE,
    STATE_TRANSITION_ERROR_EMERGENCY_MODE
} state_transition_result_t;

typedef state_transition_result_t (*state_handler_t)(bootloader_state_machine_t* sm);

typedef struct {
    bootloader_state_t state;
    state_handler_t handler;
    uint32_t default_timeout_ms;
    uint32_t warning_timeout_ms;
    bool allows_retry;
    bool critical_state;
    const char* state_name;
} state_handler_entry_t;

void bootloader_state_machine_init(bootloader_state_machine_t* sm);
void bootloader_state_machine_deinit(bootloader_state_machine_t* sm);

state_transition_result_t bootloader_state_machine_run(bootloader_state_machine_t* sm);
state_transition_result_t bootloader_state_machine_update(bootloader_state_machine_t* sm);

state_transition_result_t transition_to_state_safe(bootloader_state_machine_t* sm, bootloader_state_t new_state);
state_transition_result_t transition_to_error_state(bootloader_state_machine_t* sm, bootloader_error_code_t error_code, uint32_t context_data);

bool bootloader_state_machine_is_operational(bootloader_state_machine_t* sm);
bool bootloader_state_machine_is_error_state(bootloader_state_machine_t* sm);
bool bootloader_state_machine_can_recover(bootloader_state_machine_t* sm);

void bootloader_state_machine_set_transport(bootloader_state_machine_t* sm, const transport_interface_t* transport);
void bootloader_state_machine_set_debug_mode(bootloader_state_machine_t* sm, bool debug);
void bootloader_state_machine_set_emergency_mode(bootloader_state_machine_t* sm, bool emergency);

const char* bootloader_state_machine_get_current_state_name(bootloader_state_machine_t* sm);
uint32_t bootloader_state_machine_get_state_execution_time(bootloader_state_machine_t* sm);
uint32_t bootloader_state_machine_get_total_execution_time(bootloader_state_machine_t* sm);

state_transition_result_t handle_startup_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_trigger_detect_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_bootloader_active_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_transport_init_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_handshake_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_ready_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_receive_header_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_receive_data_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_verify_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_program_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_bank_switch_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_complete_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_communication_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_flash_operation_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_data_corruption_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_resource_exhaustion_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_timeout_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_error_hardware_fault_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_recovery_retry_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_recovery_abort_state(bootloader_state_machine_t* sm);
state_transition_result_t handle_jump_application_state(bootloader_state_machine_t* sm);

extern const state_handler_entry_t g_state_handlers[];
extern const uint8_t g_state_handler_count;

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_STATE_MACHINE_H