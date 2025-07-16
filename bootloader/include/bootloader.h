#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "bootloader_state_machine.h"
#include "bootloader_protocol.h"
#include "uart_transport.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOOTLOADER_VERSION_MAJOR 1
#define BOOTLOADER_VERSION_MINOR 0
#define BOOTLOADER_VERSION_PATCH 0

typedef struct {
    bootloader_state_machine_t state_machine;
    protocol_context_t protocol;
    
    bool initialized;
    bool debug_mode;
    bool emergency_mode;
    
    uint32_t boot_time;
    uint32_t execution_cycles;
    
    const char* version_string;
} bootloader_context_t;

typedef enum {
    BOOTLOADER_INIT_OK = 0,
    BOOTLOADER_INIT_ERROR_TRANSPORT,
    BOOTLOADER_INIT_ERROR_RESOURCE,
    BOOTLOADER_INIT_ERROR_STATE_MACHINE,
    BOOTLOADER_INIT_ERROR_PROTOCOL
} bootloader_init_result_t;

typedef enum {
    BOOTLOADER_RUN_OK = 0,
    BOOTLOADER_RUN_CONTINUE,
    BOOTLOADER_RUN_COMPLETE,
    BOOTLOADER_RUN_ERROR,
    BOOTLOADER_RUN_JUMP_APPLICATION
} bootloader_run_result_t;

bootloader_init_result_t bootloader_init(bootloader_context_t* ctx);
void bootloader_deinit(bootloader_context_t* ctx);

bootloader_run_result_t bootloader_run_cycle(bootloader_context_t* ctx);
bootloader_run_result_t bootloader_main_loop(bootloader_context_t* ctx);

void bootloader_set_debug_mode(bootloader_context_t* ctx, bool debug);
void bootloader_set_emergency_mode(bootloader_context_t* ctx, bool emergency);

bool bootloader_is_initialized(bootloader_context_t* ctx);
bool bootloader_is_ready(bootloader_context_t* ctx);
bool bootloader_can_accept_commands(bootloader_context_t* ctx);

const char* bootloader_get_version_string(bootloader_context_t* ctx);
uint32_t bootloader_get_uptime_ms(bootloader_context_t* ctx);
uint32_t bootloader_get_execution_cycles(bootloader_context_t* ctx);

void bootloader_emergency_shutdown(bootloader_context_t* ctx);

extern bootloader_context_t g_bootloader_context;

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_H