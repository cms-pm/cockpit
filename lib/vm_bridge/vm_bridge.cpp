/*
 * VM Bridge Implementation
 * Phase 4.2.1A: C++ to C Bridge Layer for STM32G431CB Hardware Integration
 */

#include "vm_bridge.h"
#include "../component_vm/include/component_vm.h"
#include "../vm_blackbox/include/vm_blackbox.h"

// Semihosting functions need extern "C" linkage for C++ bridge
extern "C" {
    #include "../semihosting/semihosting.h"
}

// C++ to C bridge implementation
extern "C" {

// Internal structure to hold C++ ComponentVM instance
struct vm_bridge_t {
    ComponentVM* vm_instance;
    vm_blackbox_t* blackbox_instance;
    bool is_valid;
    bool telemetry_enabled;
};

// Static memory allocation for VM instance (embedded-friendly)
static ComponentVM vm_storage;
static vm_bridge_t vm_handle = {nullptr, nullptr, false, false};

// Helper function to update telemetry during VM execution
static void update_telemetry_if_enabled(vm_bridge_t* vm) {
    if (!vm || !vm->telemetry_enabled || !vm->blackbox_instance) {
        return;
    }
    
    // Get VM state for telemetry
    uint32_t pc = 0; // TODO: Get actual PC from ComponentVM when available
    uint32_t instruction_count = static_cast<uint32_t>(vm->vm_instance->get_instruction_count());
    uint32_t last_opcode = 0; // TODO: Get actual last opcode when available
    
    vm_blackbox_update_execution(vm->blackbox_instance, pc, instruction_count, last_opcode);
}

vm_bridge_t* vm_bridge_create(void) {
    if (vm_handle.is_valid) {
        // WARNING: ComponentVM already created, returning existing instance
        return &vm_handle;
    }
    
    // Initialize ComponentVM in static storage
    vm_handle.vm_instance = &vm_storage;
    vm_handle.is_valid = true;
    vm_handle.telemetry_enabled = false;
    vm_handle.blackbox_instance = nullptr;
    
    // ComponentVM C bridge created successfully
    return &vm_handle;
}

void vm_bridge_destroy(vm_bridge_t* vm) {
    if (!vm || !vm->is_valid) {
        // WARNING: Invalid ComponentVM handle in destroy
        return;
    }
    
    // Cleanup telemetry if enabled
    if (vm->telemetry_enabled && vm->blackbox_instance) {
        vm_blackbox_destroy(vm->blackbox_instance);
    }
    
    // Mark as invalid (static storage cleanup not needed)
    vm->vm_instance = nullptr;
    vm->blackbox_instance = nullptr;
    vm->is_valid = false;
    vm->telemetry_enabled = false;
    
    // ComponentVM C bridge destroyed
}

vm_result_t vm_bridge_execute_program(vm_bridge_t* vm, const vm_instruction_t* program, size_t program_size) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        // ERROR: Invalid ComponentVM handle
        return VM_RESULT_ERROR;
    }
    
    if (!program || program_size == 0) {
        // ERROR: Invalid program parameters
        return VM_RESULT_ERROR;
    }
    
    // Cast C instruction format to C++ format (compatible structs)
    const VM::Instruction* cpp_program = reinterpret_cast<const VM::Instruction*>(program);
    
    // Executing program with instructions: program_size
    
    bool result = vm->vm_instance->execute_program(cpp_program, program_size);
    
    // Update telemetry after execution
    update_telemetry_if_enabled(vm);
    
    if (result) {
        // Program execution completed successfully
        return VM_RESULT_SUCCESS;
    } else {
        // Program execution failed
        return VM_RESULT_ERROR;
    }
}

vm_result_t vm_bridge_execute_single_step(vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return VM_RESULT_ERROR;
    }
    
    bool result = vm->vm_instance->execute_single_step();
    
    // Update telemetry after single step
    update_telemetry_if_enabled(vm);
    
    return result ? VM_RESULT_SUCCESS : VM_RESULT_ERROR;
}

vm_result_t vm_bridge_load_program(vm_bridge_t* vm, const vm_instruction_t* program, size_t program_size) {
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

void vm_bridge_reset(vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return;
    }
    
    vm->vm_instance->reset_vm();
    // ComponentVM reset completed
}

bool vm_bridge_is_running(const vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return false;
    }
    
    return vm->vm_instance->is_running();
}

bool vm_bridge_is_halted(const vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return true;
    }
    
    return vm->vm_instance->is_halted();
}

size_t vm_bridge_get_instruction_count(const vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return 0;
    }
    
    return vm->vm_instance->get_instruction_count();
}

vm_performance_metrics_t vm_bridge_get_performance_metrics(const vm_bridge_t* vm) {
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

void vm_bridge_reset_performance_metrics(vm_bridge_t* vm) {
    if (!vm || !vm->is_valid || !vm->vm_instance) {
        return;
    }
    
    vm->vm_instance->reset_performance_metrics();
}

const char* vm_bridge_get_error_string(vm_result_t result) {
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

// Phase 4.2.2B: Telemetry integration functions
void vm_bridge_enable_telemetry(vm_bridge_t* vm, bool enable) {
    if (!vm || !vm->is_valid) {
        // ERROR: Invalid ComponentVM handle for telemetry
        return;
    }
    
    if (enable && !vm->telemetry_enabled) {
        // Initialize blackbox for telemetry
        vm->blackbox_instance = vm_blackbox_create();
        if (vm->blackbox_instance) {
            vm->telemetry_enabled = true;
            // ComponentVM telemetry enabled
            
            // Initialize with current VM state
            update_telemetry_if_enabled(vm);
        } else {
            // ERROR: Failed to create blackbox instance
        }
    } else if (!enable && vm->telemetry_enabled) {
        // Disable telemetry
        if (vm->blackbox_instance) {
            vm_blackbox_destroy(vm->blackbox_instance);
            vm->blackbox_instance = nullptr;
        }
        vm->telemetry_enabled = false;
        // ComponentVM telemetry disabled
    }
}

bool vm_bridge_is_telemetry_enabled(const vm_bridge_t* vm) {
    if (!vm || !vm->is_valid) {
        return false;
    }
    
    return vm->telemetry_enabled;
}

} // extern "C"