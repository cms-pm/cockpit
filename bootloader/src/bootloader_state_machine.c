#include "bootloader_state_machine.h"
#include <string.h>

const state_handler_entry_t g_state_handlers[] = {
    {STATE_STARTUP, handle_startup_state, 1000, 800, false, false, "STARTUP"},
    {STATE_TRIGGER_DETECT, handle_trigger_detect_state, 5000, 4000, true, false, "TRIGGER_DETECT"},
    {STATE_BOOTLOADER_ACTIVE, handle_bootloader_active_state, 2000, 1500, false, false, "BOOTLOADER_ACTIVE"},
    {STATE_TRANSPORT_INIT, handle_transport_init_state, 3000, 2000, true, false, "TRANSPORT_INIT"},
    {STATE_HANDSHAKE, handle_handshake_state, 10000, 7000, true, false, "HANDSHAKE"},
    {STATE_READY, handle_ready_state, 30000, 25000, false, false, "READY"},
    {STATE_RECEIVE_HEADER, handle_receive_header_state, 15000, 12000, true, false, "RECEIVE_HEADER"},
    {STATE_RECEIVE_DATA, handle_receive_data_state, 60000, 50000, true, false, "RECEIVE_DATA"},
    {STATE_VERIFY, handle_verify_state, 5000, 4000, true, true, "VERIFY"},
    {STATE_PROGRAM, handle_program_state, 30000, 25000, true, true, "PROGRAM"},
    {STATE_BANK_SWITCH, handle_bank_switch_state, 10000, 8000, true, true, "BANK_SWITCH"},
    {STATE_COMPLETE, handle_complete_state, 2000, 1500, false, false, "COMPLETE"},
    {STATE_ERROR_COMMUNICATION, handle_error_communication_state, 5000, 4000, true, false, "ERROR_COMMUNICATION"},
    {STATE_ERROR_FLASH_OPERATION, handle_error_flash_operation_state, 5000, 4000, true, true, "ERROR_FLASH_OPERATION"},
    {STATE_ERROR_DATA_CORRUPTION, handle_error_data_corruption_state, 5000, 4000, true, false, "ERROR_DATA_CORRUPTION"},
    {STATE_ERROR_RESOURCE_EXHAUSTION, handle_error_resource_exhaustion_state, 5000, 4000, true, true, "ERROR_RESOURCE_EXHAUSTION"},
    {STATE_ERROR_TIMEOUT, handle_error_timeout_state, 5000, 4000, true, false, "ERROR_TIMEOUT"},
    {STATE_ERROR_HARDWARE_FAULT, handle_error_hardware_fault_state, 5000, 4000, false, true, "ERROR_HARDWARE_FAULT"},
    {STATE_RECOVERY_RETRY, handle_recovery_retry_state, 3000, 2000, false, false, "RECOVERY_RETRY"},
    {STATE_RECOVERY_ABORT, handle_recovery_abort_state, 2000, 1500, false, false, "RECOVERY_ABORT"},
    {STATE_JUMP_APPLICATION, handle_jump_application_state, 1000, 800, false, false, "JUMP_APPLICATION"}
};

const uint8_t g_state_handler_count = sizeof(g_state_handlers) / sizeof(state_handler_entry_t);

static const state_handler_entry_t* get_state_handler(bootloader_state_t state) {
    for (uint8_t i = 0; i < g_state_handler_count; i++) {
        if (g_state_handlers[i].state == state) {
            return &g_state_handlers[i];
        }
    }
    return NULL;
}

static void cleanup_state_resources(bootloader_state_machine_t* sm, bootloader_state_t state) {
    if (!sm || !sm->resource_manager) return;
    
    switch (state) {
        case STATE_TRANSPORT_INIT:
            resource_manager_cleanup_by_type(sm->resource_manager, RESOURCE_TYPE_TRANSPORT);
            break;
        case STATE_PROGRAM:
            resource_manager_cleanup_by_type(sm->resource_manager, RESOURCE_TYPE_FLASH);
            break;
        case STATE_RECEIVE_DATA:
            resource_manager_cleanup_by_type(sm->resource_manager, RESOURCE_TYPE_BUFFER);
            break;
        default:
            break;
    }
}

static state_transition_result_t initialize_state_resources(bootloader_state_machine_t* sm, bootloader_state_t state) {
    if (!sm || !sm->resource_manager) {
        return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    }
    
    switch (state) {
        case STATE_TRANSPORT_INIT:
            if (transport_is_initialized(&sm->transport)) {
                return STATE_TRANSITION_OK;
            }
            break;
        case STATE_PROGRAM:
            break;
        case STATE_RECEIVE_DATA:
            break;
        default:
            break;
    }
    
    return STATE_TRANSITION_OK;
}

void bootloader_state_machine_init(bootloader_state_machine_t* sm) {
    if (!sm) return;
    
    memset(sm, 0, sizeof(bootloader_state_machine_t));
    
    sm->current_state = STATE_STARTUP;
    sm->next_state = STATE_STARTUP;
    sm->previous_state = STATE_STARTUP;
    
    sm->state_entry_time = get_system_tick_safe();
    sm->max_state_retries = 3;
    
    timeout_configure(&sm->state_timeout, 1000, 800, 3);
    timeout_configure(&sm->operation_timeout, 5000, 4000, 3);
    
    sm->error_manager = &g_error_manager;
    sm->resource_manager = &g_resource_manager;
    sm->timeout_manager = &g_timeout_manager;
    
    error_manager_init(sm->error_manager);
    resource_manager_init(sm->resource_manager);
    timeout_manager_init(sm->timeout_manager);
    
    g_current_state = sm->current_state;
}

void bootloader_state_machine_deinit(bootloader_state_machine_t* sm) {
    if (!sm) return;
    
    resource_manager_emergency_cleanup(sm->resource_manager);
    transport_deinit(&sm->transport);
    
    memset(sm, 0, sizeof(bootloader_state_machine_t));
}

state_transition_result_t transition_to_state_safe(bootloader_state_machine_t* sm, bootloader_state_t new_state) {
    if (!sm) {
        return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    }
    
    if (sm->emergency_mode && !bootloader_state_machine_is_error_state(sm)) {
        return STATE_TRANSITION_ERROR_EMERGENCY_MODE;
    }
    
    cleanup_state_resources(sm, sm->current_state);
    
    state_transition_result_t result = initialize_state_resources(sm, new_state);
    if (result != STATE_TRANSITION_OK) {
        transition_to_error_state(sm, ERROR_RESOURCE_LOCKED, (uint32_t)new_state);
        return result;
    }
    
    sm->previous_state = sm->current_state;
    sm->current_state = new_state;
    sm->next_state = new_state;
    sm->state_entry_time = get_system_tick_safe();
    sm->state_execution_count = 0;
    sm->state_transition_count++;
    sm->state_change_pending = false;
    
    g_current_state = sm->current_state;
    
    const state_handler_entry_t* handler = get_state_handler(new_state);
    if (handler) {
        timeout_configure(&sm->state_timeout, handler->default_timeout_ms, 
                         handler->warning_timeout_ms, sm->max_state_retries);
        timeout_start(&sm->state_timeout);
    }
    
    if (sm->debug_mode) {
        LOG_ERROR(ERROR_NONE, ERROR_SEVERITY_INFO, (uint32_t)new_state, 
                 "State transition");
    }
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t transition_to_error_state(bootloader_state_machine_t* sm, bootloader_error_code_t error_code, uint32_t context_data) {
    if (!sm) {
        return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    }
    
    bootloader_state_t error_state = error_code_to_state(error_code);
    
    LOG_ERROR(error_code, ERROR_SEVERITY_ERROR, context_data, "Transitioning to error state");
    
    return transition_to_state_safe(sm, error_state);
}

state_transition_result_t bootloader_state_machine_run(bootloader_state_machine_t* sm) {
    if (!sm) {
        return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    }
    
    const state_handler_entry_t* handler = get_state_handler(sm->current_state);
    if (!handler || !handler->handler) {
        return transition_to_error_state(sm, ERROR_STATE_VIOLATION, (uint32_t)sm->current_state);
    }
    
    sm->state_execution_count++;
    
    timeout_manager_record_activity(sm->timeout_manager);
    
    state_transition_result_t result = handler->handler(sm);
    
    if (timeout_is_expired(&sm->state_timeout)) {
        if (handler->allows_retry && timeout_can_retry(&sm->state_timeout)) {
            timeout_retry(&sm->state_timeout);
            LOG_TIMEOUT_ERROR(ERROR_OPERATION_TIMEOUT, (uint32_t)sm->current_state);
        } else {
            return transition_to_error_state(sm, ERROR_OPERATION_TIMEOUT, (uint32_t)sm->current_state);
        }
    }
    
    if (result != STATE_TRANSITION_OK && handler->allows_retry && 
        sm->state_retry_count < sm->max_state_retries) {
        sm->state_retry_count++;
        result = STATE_TRANSITION_OK;
        
        if (sm->debug_mode) {
            LOG_ERROR(ERROR_NONE, ERROR_SEVERITY_WARNING, sm->state_retry_count, 
                     "State retry");
        }
    } else if (result != STATE_TRANSITION_OK) {
        return transition_to_error_state(sm, ERROR_STATE_VIOLATION, (uint32_t)result);
    } else {
        sm->state_retry_count = 0;
    }
    
    if (sm->state_change_pending) {
        return transition_to_state_safe(sm, sm->next_state);
    }
    
    return result;
}

state_transition_result_t bootloader_state_machine_update(bootloader_state_machine_t* sm) {
    if (!sm) {
        return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    }
    
    timeout_manager_update(sm->timeout_manager);
    
    if (resource_manager_has_error_resources(sm->resource_manager)) {
        return transition_to_error_state(sm, ERROR_RESOURCE_LOCKED, 0);
    }
    
    return STATE_TRANSITION_OK;
}

bool bootloader_state_machine_is_operational(bootloader_state_machine_t* sm) {
    if (!sm) return false;
    
    return sm->current_state >= STATE_STARTUP && sm->current_state <= STATE_COMPLETE;
}

bool bootloader_state_machine_is_error_state(bootloader_state_machine_t* sm) {
    if (!sm) return false;
    
    return sm->current_state >= STATE_ERROR_COMMUNICATION && 
           sm->current_state <= STATE_ERROR_HARDWARE_FAULT;
}

bool bootloader_state_machine_can_recover(bootloader_state_machine_t* sm) {
    if (!sm) return false;
    
    const state_handler_entry_t* handler = get_state_handler(sm->current_state);
    return handler && handler->allows_retry && !handler->critical_state;
}

void bootloader_state_machine_set_transport(bootloader_state_machine_t* sm, const transport_interface_t* transport) {
    if (!sm || !transport) return;
    
    transport_init(&sm->transport, transport);
}

void bootloader_state_machine_set_debug_mode(bootloader_state_machine_t* sm, bool debug) {
    if (!sm) return;
    
    sm->debug_mode = debug;
}

void bootloader_state_machine_set_emergency_mode(bootloader_state_machine_t* sm, bool emergency) {
    if (!sm) return;
    
    sm->emergency_mode = emergency;
    resource_manager_set_emergency_mode(sm->resource_manager, emergency);
}

const char* bootloader_state_machine_get_current_state_name(bootloader_state_machine_t* sm) {
    if (!sm) return "INVALID";
    
    const state_handler_entry_t* handler = get_state_handler(sm->current_state);
    return handler ? handler->state_name : "UNKNOWN";
}

uint32_t bootloader_state_machine_get_state_execution_time(bootloader_state_machine_t* sm) {
    if (!sm) return 0;
    
    return get_system_tick_safe() - sm->state_entry_time;
}

uint32_t bootloader_state_machine_get_total_execution_time(bootloader_state_machine_t* sm) {
    if (!sm) return 0;
    
    return sm->state_transition_count * 100;
}

state_transition_result_t handle_startup_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_TRIGGER_DETECT;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_trigger_detect_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_BOOTLOADER_ACTIVE;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_bootloader_active_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_TRANSPORT_INIT;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_transport_init_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    if (!transport_is_initialized(&sm->transport)) {
        return STATE_TRANSITION_ERROR_RESOURCE_BUSY;
    }
    
    sm->next_state = STATE_HANDSHAKE;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_handshake_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_READY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_ready_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_receive_header_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECEIVE_DATA;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_receive_data_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_VERIFY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_verify_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_PROGRAM;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_program_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_BANK_SWITCH;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_bank_switch_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_COMPLETE;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_complete_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_JUMP_APPLICATION;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_communication_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECOVERY_RETRY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_flash_operation_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECOVERY_ABORT;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_data_corruption_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECOVERY_RETRY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_resource_exhaustion_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    resource_manager_emergency_cleanup(sm->resource_manager);
    
    sm->next_state = STATE_RECOVERY_RETRY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_timeout_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECOVERY_RETRY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_error_hardware_fault_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_RECOVERY_ABORT;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_recovery_retry_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_READY;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_recovery_abort_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    sm->next_state = STATE_JUMP_APPLICATION;
    sm->state_change_pending = true;
    
    return STATE_TRANSITION_OK;
}

state_transition_result_t handle_jump_application_state(bootloader_state_machine_t* sm) {
    if (!sm) return STATE_TRANSITION_ERROR_CRITICAL_FAILURE;
    
    resource_manager_cleanup_all(sm->resource_manager);
    
    return STATE_TRANSITION_OK;
}