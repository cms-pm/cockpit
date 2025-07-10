/*
 * ComponentVM C Wrapper Implementation
 * Phase 3: C-compatible wrapper for C++ ComponentVM implementation
 */

#include "component_vm_c.h"
#include "component_vm.h"
#include <new>
#include <cstring>

// Opaque C struct that contains the C++ ComponentVM instance
struct ComponentVM_C {
    ComponentVM* vm_instance;
    
    ComponentVM_C() : vm_instance(nullptr) {}
    ~ComponentVM_C() {
        if (vm_instance) {
            delete vm_instance;
            vm_instance = nullptr;
        }
    }
};

// Helper function to convert C instruction to C++ format
static VM::Instruction convert_instruction(const vm_instruction_c_t& c_instr) {
    VM::Instruction cpp_instr;
    cpp_instr.opcode = c_instr.opcode;
    cpp_instr.flags = c_instr.flags;
    cpp_instr.immediate = c_instr.immediate;
    return cpp_instr;
}

// No more error conversion needed - unified error system
// ComponentVM will be updated to use vm_error_t directly

// === Core VM Functions Implementation ===

ComponentVM_C* component_vm_create(void) {
    ComponentVM_C* c_vm = new(std::nothrow) ComponentVM_C();
    if (!c_vm) {
        return nullptr;
    }
    
    c_vm->vm_instance = new(std::nothrow) ComponentVM();
    if (!c_vm->vm_instance) {
        delete c_vm;
        return nullptr;
    }
    
    return c_vm;
}

void component_vm_destroy(ComponentVM_C* vm) {
    if (vm) {
        delete vm;
    }
}

bool component_vm_execute_program(ComponentVM_C* vm, const vm_instruction_c_t* program, size_t program_size) {
    if (!vm || !vm->vm_instance || !program) {
        return false;
    }
    
    // Convert C instructions to C++ format
    VM::Instruction* cpp_program = new(std::nothrow) VM::Instruction[program_size];
    if (!cpp_program) {
        return false;
    }
    
    for (size_t i = 0; i < program_size; i++) {
        cpp_program[i] = convert_instruction(program[i]);
    }
    
    bool result = vm->vm_instance->execute_program(cpp_program, program_size);
    
    delete[] cpp_program;
    return result;
}

bool component_vm_load_program(ComponentVM_C* vm, const vm_instruction_c_t* program, size_t program_size) {
    if (!vm || !vm->vm_instance) {
        return false;
    }
    
    // Let the C++ ComponentVM handle null program validation and error setting
    if (!program || program_size == 0) {
        return vm->vm_instance->load_program(nullptr, 0);
    }
    
    // Convert C instructions to C++ format
    VM::Instruction* cpp_program = new(std::nothrow) VM::Instruction[program_size];
    if (!cpp_program) {
        return false;
    }
    
    for (size_t i = 0; i < program_size; i++) {
        cpp_program[i] = convert_instruction(program[i]);
    }
    
    bool result = vm->vm_instance->load_program(cpp_program, program_size);
    
    delete[] cpp_program;
    return result;
}

bool component_vm_execute_single_step(ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return false;
    }
    
    return vm->vm_instance->execute_single_step();
}

void component_vm_reset(ComponentVM_C* vm) {
    if (vm && vm->vm_instance) {
        vm->vm_instance->reset_vm();
    }
}

// === VM State Inspection ===

bool component_vm_is_running(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return false;
    }
    
    return vm->vm_instance->is_running();
}

bool component_vm_is_halted(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return false;
    }
    
    return vm->vm_instance->is_halted();
}

size_t component_vm_get_instruction_count(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return 0;
    }
    
    return vm->vm_instance->get_instruction_count();
}

// === Error Handling ===

vm_error_t component_vm_get_last_error(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return VM_ERROR_PROGRAM_NOT_LOADED;
    }
    
    // ComponentVM now uses unified error system directly
    return vm->vm_instance->get_last_error();
}

const char* component_vm_get_error_string(vm_error_t error) {
    // Use unified error system string conversion
    return vm_error_to_string(error);
}

// === Performance Monitoring ===

vm_c_performance_metrics_t component_vm_get_performance_metrics(const ComponentVM_C* vm) {
    vm_c_performance_metrics_t c_metrics = {0, 0, 0, 0};
    
    if (vm && vm->vm_instance) {
        ComponentVM::PerformanceMetrics cpp_metrics = vm->vm_instance->get_performance_metrics();
        c_metrics.execution_time_ms = cpp_metrics.execution_time_ms;
        c_metrics.instructions_executed = cpp_metrics.instructions_executed;
        c_metrics.memory_operations = cpp_metrics.memory_operations;
        c_metrics.io_operations = cpp_metrics.io_operations;
    }
    
    return c_metrics;
}

void component_vm_reset_performance_metrics(ComponentVM_C* vm) {
    if (vm && vm->vm_instance) {
        vm->vm_instance->reset_performance_metrics();
    }
}

// Legacy compatibility functions removed - wisteria eradicated

// ============= TIER 1 STATE VALIDATION IMPLEMENTATION =============
// The Golden Triangle: Stack, Memory, Execution validation with canary integration

bool component_vm_validate_memory_integrity(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return false;
    }
    
    // Validate memory manager integrity
    return vm->vm_instance->get_memory_manager().validate_memory_integrity();
}

size_t component_vm_get_stack_pointer(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return 0;
    }
    
    return vm->vm_instance->get_execution_engine().get_sp();
}

size_t component_vm_get_program_counter(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return 0;
    }
    
    return vm->vm_instance->get_execution_engine().get_pc();
}

bool component_vm_validate_stack_state(const ComponentVM_C* vm,
                                       const vm_stack_validation_t* expected_stack) {
    if (!vm || !vm->vm_instance || !expected_stack) {
        return false;
    }
    
    const ExecutionEngine& engine = vm->vm_instance->get_execution_engine();
    
    // Validate stack pointer
    size_t actual_sp = engine.get_sp();
    if (actual_sp != expected_stack->expected_sp) {
        return false;  // Stack pointer mismatch
    }
    
    // Validate clean stack expectation
    if (expected_stack->stack_should_be_clean && actual_sp != 1) {
        return false;  // Stack should be clean but isn't
    }
    
    // Validate canary integrity if requested
    if (expected_stack->canaries_should_be_intact) {
        #ifdef DEBUG
        // Access canary validation through protected method
        // Note: This requires friendship or protected access
        // For now, we'll validate through memory integrity
        if (!component_vm_validate_memory_integrity(vm)) {
            return false;  // Canaries died - memory corruption
        }
        #endif
    }
    
    // TODO: Validate expected_top_values when stack inspection is implemented
    // This requires additional API to peek at stack contents
    
    return true;
}

bool component_vm_validate_memory_state(const ComponentVM_C* vm,
                                        const vm_memory_expectation_t* expectations,
                                        size_t count) {
    if (!vm || !vm->vm_instance || !expectations) {
        return false;
    }
    
    const MemoryManager& memory = vm->vm_instance->get_memory_manager();
    
    // Validate each memory expectation
    for (size_t i = 0; i < count; i++) {
        const vm_memory_expectation_t* expectation = &expectations[i];
        
        int32_t actual_value;
        if (!memory.load_global(expectation->variable_index, actual_value)) {
            return false;  // Failed to load global variable
        }
        
        if (actual_value != expectation->expected_value) {
            return false;  // Value mismatch
        }
    }
    
    return true;
}

bool component_vm_validate_final_state(const ComponentVM_C* vm, 
                                       const vm_final_state_validation_t* expected_state) {
    if (!vm || !vm->vm_instance || !expected_state) {
        return false;
    }
    
    const ExecutionEngine& engine = vm->vm_instance->get_execution_engine();
    
    // Validate execution state
    const vm_execution_validation_t* exec_validation = &expected_state->execution_validation;
    
    // Check halt state
    if (engine.is_halted() != exec_validation->should_be_halted) {
        return false;  // Halt state mismatch
    }
    
    // Check program counter
    if (engine.get_pc() != exec_validation->expected_final_pc) {
        return false;  // Program counter mismatch
    }
    
    // Check instruction count
    size_t actual_instruction_count = component_vm_get_instruction_count(vm);
    if (actual_instruction_count != exec_validation->expected_instruction_count) {
        return false;  // Instruction count mismatch
    }
    
    // Validate stack state
    if (!component_vm_validate_stack_state(vm, &expected_state->stack_validation)) {
        return false;  // Stack validation failed
    }
    
    // Validate memory state
    if (expected_state->memory_checks && expected_state->memory_check_count > 0) {
        if (!component_vm_validate_memory_state(vm, expected_state->memory_checks, 
                                               expected_state->memory_check_count)) {
            return false;  // Memory validation failed
        }
    }
    
    return true;  // All validations passed - canaries are singing!
}
