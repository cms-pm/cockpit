#include "bootloader.h"
#include <string.h>
#include <stdio.h>

bootloader_context_t g_bootloader_context = {0};

static char g_version_string[32] = {0};

bootloader_init_result_t bootloader_init(bootloader_context_t* ctx) {
    if (!ctx) return BOOTLOADER_INIT_ERROR_RESOURCE;
    
    memset(ctx, 0, sizeof(bootloader_context_t));
    
    snprintf(g_version_string, sizeof(g_version_string), "BOOTLOADER v%d.%d.%d", 
             BOOTLOADER_VERSION_MAJOR, BOOTLOADER_VERSION_MINOR, BOOTLOADER_VERSION_PATCH);
    ctx->version_string = g_version_string;
    
    bootloader_state_machine_init(&ctx->state_machine);
    
    bootloader_state_machine_set_transport(&ctx->state_machine, &uart_transport_interface);
    
    protocol_init(&ctx->protocol, &ctx->state_machine.transport);
    
    ctx->boot_time = get_system_tick_safe();
    ctx->initialized = true;
    
    return BOOTLOADER_INIT_OK;
}

void bootloader_deinit(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    protocol_deinit(&ctx->protocol);
    bootloader_state_machine_deinit(&ctx->state_machine);
    
    memset(ctx, 0, sizeof(bootloader_context_t));
}

bootloader_run_result_t bootloader_run_cycle(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) {
        return BOOTLOADER_RUN_ERROR;
    }
    
    ctx->execution_cycles++;
    
    state_transition_result_t sm_result = bootloader_state_machine_run(&ctx->state_machine);
    
    if (sm_result != STATE_TRANSITION_OK) {
        if (ctx->debug_mode) {
            LOG_ERROR(ERROR_STATE_VIOLATION, ERROR_SEVERITY_ERROR, 
                     (uint32_t)sm_result, "State machine error");
        }
        return BOOTLOADER_RUN_ERROR;
    }
    
    bootloader_state_machine_update(&ctx->state_machine);
    
    if (ctx->state_machine.current_state == STATE_JUMP_APPLICATION) {
        return BOOTLOADER_RUN_JUMP_APPLICATION;
    }
    
    if (bootloader_state_machine_is_error_state(&ctx->state_machine)) {
        if (bootloader_state_machine_can_recover(&ctx->state_machine)) {
            return BOOTLOADER_RUN_CONTINUE;
        } else {
            return BOOTLOADER_RUN_ERROR;
        }
    }
    
    if (ctx->state_machine.current_state == STATE_READY ||
        ctx->state_machine.current_state == STATE_HANDSHAKE ||
        ctx->state_machine.current_state == STATE_RECEIVE_HEADER ||
        ctx->state_machine.current_state == STATE_RECEIVE_DATA) {
        
        protocol_response_t protocol_result = protocol_process_message(&ctx->protocol);
        
        if (protocol_result == RESP_ERROR_HARDWARE) {
            return BOOTLOADER_RUN_ERROR;
        }
        
        if (protocol_result == RESP_UPLOAD_SUCCESS) {
            transition_to_state_safe(&ctx->state_machine, STATE_VERIFY);
        }
    }
    
    if (ctx->state_machine.current_state == STATE_COMPLETE) {
        return BOOTLOADER_RUN_COMPLETE;
    }
    
    return BOOTLOADER_RUN_CONTINUE;
}

bootloader_run_result_t bootloader_main_loop(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) {
        return BOOTLOADER_RUN_ERROR;
    }
    
    bootloader_run_result_t result;
    
    do {
        result = bootloader_run_cycle(ctx);
        
        if (ctx->emergency_mode && result == BOOTLOADER_RUN_ERROR) {
            bootloader_emergency_shutdown(ctx);
            break;
        }
        
    } while (result == BOOTLOADER_RUN_CONTINUE);
    
    return result;
}

void bootloader_set_debug_mode(bootloader_context_t* ctx, bool debug) {
    if (!ctx) return;
    
    ctx->debug_mode = debug;
    bootloader_state_machine_set_debug_mode(&ctx->state_machine, debug);
    protocol_set_debug(&ctx->protocol, debug);
}

void bootloader_set_emergency_mode(bootloader_context_t* ctx, bool emergency) {
    if (!ctx) return;
    
    ctx->emergency_mode = emergency;
    bootloader_state_machine_set_emergency_mode(&ctx->state_machine, emergency);
}

bool bootloader_is_initialized(bootloader_context_t* ctx) {
    return ctx && ctx->initialized;
}

bool bootloader_is_ready(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) return false;
    
    return bootloader_state_machine_is_operational(&ctx->state_machine) &&
           protocol_is_ready(&ctx->protocol);
}

bool bootloader_can_accept_commands(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) return false;
    
    return ctx->state_machine.current_state == STATE_READY ||
           ctx->state_machine.current_state == STATE_HANDSHAKE ||
           ctx->state_machine.current_state == STATE_RECEIVE_HEADER ||
           ctx->state_machine.current_state == STATE_RECEIVE_DATA;
}

const char* bootloader_get_version_string(bootloader_context_t* ctx) {
    if (!ctx) return "UNKNOWN";
    
    return ctx->version_string ? ctx->version_string : "BOOTLOADER v1.0.0";
}

uint32_t bootloader_get_uptime_ms(bootloader_context_t* ctx) {
    if (!ctx || !ctx->initialized) return 0;
    
    return get_system_tick_safe() - ctx->boot_time;
}

uint32_t bootloader_get_execution_cycles(bootloader_context_t* ctx) {
    if (!ctx) return 0;
    
    return ctx->execution_cycles;
}

void bootloader_emergency_shutdown(bootloader_context_t* ctx) {
    if (!ctx) return;
    
    bootloader_set_emergency_mode(ctx, true);
    
    resource_manager_emergency_cleanup(&g_resource_manager);
    
    transport_deinit(&ctx->state_machine.transport);
    
    transition_to_state_safe(&ctx->state_machine, STATE_JUMP_APPLICATION);
    
    if (ctx->debug_mode) {
        LOG_ERROR(ERROR_HARDWARE_FAULT, ERROR_SEVERITY_CRITICAL, 0, 
                 "Emergency shutdown");
    }
}