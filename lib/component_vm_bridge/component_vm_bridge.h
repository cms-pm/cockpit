/*
 * ComponentVM C Bridge Interface
 * Phase 4.2.1A: C++ to C Bridge Layer for STM32G431CB Hardware Integration
 * 
 * Provides C-compatible interface to ComponentVM C++ implementation
 * Thin wrapper design for minimal overhead
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations for C compatibility
typedef struct component_vm_t component_vm_t;

// VM instruction structure (matches C++ VM::Instruction)
typedef struct {
    uint8_t  opcode;     // 256 base operations
    uint8_t  flags;      // 8 modifier bits
    uint16_t immediate;  // 0-65535 range
} vm_instruction_t;

// VM execution result codes
typedef enum {
    VM_RESULT_SUCCESS = 0,
    VM_RESULT_ERROR = 1,
    VM_RESULT_HALTED = 2,
    VM_RESULT_MEMORY_ERROR = 3,
    VM_RESULT_INVALID_INSTRUCTION = 4
} vm_result_t;

// VM performance metrics
typedef struct {
    uint32_t execution_time_ms;
    size_t instructions_executed;
    size_t memory_operations;
    size_t io_operations;
} vm_performance_metrics_t;

// VM lifecycle management
component_vm_t* component_vm_create(void);
void component_vm_destroy(component_vm_t* vm);

// VM execution functions
vm_result_t component_vm_execute_program(component_vm_t* vm, const vm_instruction_t* program, size_t program_size);
vm_result_t component_vm_execute_single_step(component_vm_t* vm);
vm_result_t component_vm_load_program(component_vm_t* vm, const vm_instruction_t* program, size_t program_size);
void component_vm_reset(component_vm_t* vm);

// VM state inspection
bool component_vm_is_running(const component_vm_t* vm);
bool component_vm_is_halted(const component_vm_t* vm);
size_t component_vm_get_instruction_count(const component_vm_t* vm);

// Performance monitoring
vm_performance_metrics_t component_vm_get_performance_metrics(const component_vm_t* vm);
void component_vm_reset_performance_metrics(component_vm_t* vm);

// Error handling
const char* component_vm_get_error_string(vm_result_t result);

#ifdef __cplusplus
}
#endif