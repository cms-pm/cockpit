/*
 * ComponentVM C Bridge Implementation
 * Phase 4.2.1A: C++ to C Bridge Layer for STM32G431CB Hardware Integration
 */

#include "component_vm_bridge.h"
#include "../component_vm/include/component_vm.h"
#include "../semihosting/semihosting.h"

// C++ to C bridge implementation
extern "C" {

// Internal structure to hold C++ ComponentVM instance
struct component_vm_t {
    ComponentVM* vm_instance;
    bool is_valid;
};

// Static memory allocation for VM instance (embedded-friendly)
static ComponentVM vm_storage;
static component_vm_t vm_handle = {nullptr, false};

component_vm_t* component_vm_create(void) {
    if (vm_handle.is_valid) {
        debug_print("WARNING: ComponentVM already created, returning existing instance");
        return &vm_handle;
    }
    
    // Initialize ComponentVM in static storage
    vm_handle.vm_instance = &vm_storage;
    vm_handle.is_valid = true;
    
    debug_print("ComponentVM C bridge created successfully");
    return &vm_handle;
}

void component_vm_destroy(component_vm_t* vm) {
    if (!vm || !vm->is_valid) {
        debug_print("WARNING: Invalid ComponentVM handle in destroy");
        return;
    }
    
    // Mark as invalid (static storage cleanup not needed)
    vm->vm_instance = nullptr;
    vm->is_valid = false;
    
    debug_print("ComponentVM C bridge destroyed");
}

vm_result_t component_vm_execute_program(component_vm_t* vm, const vm_instruction_t* program, size_t program_size) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        debug_print("ERROR: Invalid ComponentVM handle");
        return VM_RESULT_ERROR;
    }
    
    if (!program || program_size == 0) {
        debug_print("ERROR: Invalid program parameters");
        return VM_RESULT_ERROR;
    }
    
    // Cast C instruction format to C++ format (compatible structs)
    const VM::Instruction* cpp_program = reinterpret_cast<const VM::Instruction*>(program);
    
    debug_print_dec("Executing program with instructions", program_size);
    
    bool result = vm->vm_instance->execute_program(cpp_program, program_size);
    
    if (result) {
        debug_print("Program execution completed successfully");
        return VM_RESULT_SUCCESS;
    } else {
        debug_print("Program execution failed");
        return VM_RESULT_ERROR;
    }
}

vm_result_t component_vm_execute_single_step(component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return VM_RESULT_ERROR;
    }
    
    bool result = vm->vm_instance->execute_single_step();
    return result ? VM_RESULT_SUCCESS : VM_RESULT_ERROR;
}

vm_result_t component_vm_load_program(component_vm_t* vm, const vm_instruction_t* program, size_t program_size) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return VM_RESULT_ERROR;
    }
    
    if (!program || program_size == 0) {
        return VM_RESULT_ERROR;
    }
    
    const VM::Instruction* cpp_program = reinterpret_cast<const VM::Instruction*>(program);
    
    bool result = vm->vm_instance->load_program(cpp_program, program_size);
    return result ? VM_RESULT_SUCCESS : VM_RESULT_ERROR;
}

void component_vm_reset(component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return;
    }
    
    vm->vm_instance->reset_vm();
    debug_print("ComponentVM reset completed");
}

bool component_vm_is_running(const component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return false;
    }
    
    return vm->vm_instance->is_running();
}

bool component_vm_is_halted(const component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return true;
    }
    
    return vm->vm_instance->is_halted();
}

size_t component_vm_get_instruction_count(const component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return 0;
    }
    
    return vm->vm_instance->get_instruction_count();
}

vm_performance_metrics_t component_vm_get_performance_metrics(const component_vm_t* vm) {
    vm_performance_metrics_t metrics = {0, 0, 0, 0};
    
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return metrics;
    }
    
    ComponentVM::PerformanceMetrics cpp_metrics = vm->vm_instance->get_performance_metrics();
    
    metrics.execution_time_ms = cpp_metrics.execution_time_ms;
    metrics.instructions_executed = cpp_metrics.instructions_executed;
    metrics.memory_operations = cpp_metrics.memory_operations;
    metrics.io_operations = cpp_metrics.io_operations;
    
    return metrics;
}

void component_vm_reset_performance_metrics(component_vm_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return;
    }
    
    vm->vm_instance->reset_performance_metrics();
}

const char* component_vm_get_error_string(vm_result_t result) {
    switch (result) {
        case VM_RESULT_SUCCESS:
            return "Success";
        case VM_RESULT_ERROR:
            return "General error";
        case VM_RESULT_HALTED:
            return "VM halted";
        case VM_RESULT_MEMORY_ERROR:
            return "Memory error";
        case VM_RESULT_INVALID_INSTRUCTION:
            return "Invalid instruction";
        default:
            return "Unknown error";
    }
}

} // extern "C"