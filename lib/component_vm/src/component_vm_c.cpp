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

// Helper function to convert C++ error to C error
static vm_c_error_t convert_error(ComponentVM::VMError cpp_error) {
    switch (cpp_error) {
        case ComponentVM::VMError::NONE:                return VM_C_ERROR_NONE;
        case ComponentVM::VMError::STACK_OVERFLOW:      return VM_C_ERROR_STACK_OVERFLOW;
        case ComponentVM::VMError::STACK_UNDERFLOW:     return VM_C_ERROR_STACK_UNDERFLOW;
        case ComponentVM::VMError::INVALID_INSTRUCTION: return VM_C_ERROR_INVALID_INSTRUCTION;
        case ComponentVM::VMError::MEMORY_BOUNDS_ERROR: return VM_C_ERROR_MEMORY_BOUNDS_ERROR;
        case ComponentVM::VMError::IO_ERROR:            return VM_C_ERROR_IO_ERROR;
        case ComponentVM::VMError::PROGRAM_NOT_LOADED:  return VM_C_ERROR_PROGRAM_NOT_LOADED;
        default:                                        return VM_C_ERROR_INVALID_INSTRUCTION;
    }
}

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

vm_c_error_t component_vm_get_last_error(const ComponentVM_C* vm) {
    if (!vm || !vm->vm_instance) {
        return VM_C_ERROR_PROGRAM_NOT_LOADED;
    }
    
    ComponentVM::VMError cpp_error = vm->vm_instance->get_last_error();
    return convert_error(cpp_error);
}

const char* component_vm_get_error_string(vm_c_error_t error) {
    switch (error) {
        case VM_C_ERROR_NONE:                return "No error";
        case VM_C_ERROR_STACK_OVERFLOW:      return "Stack overflow";
        case VM_C_ERROR_STACK_UNDERFLOW:     return "Stack underflow";
        case VM_C_ERROR_INVALID_INSTRUCTION: return "Invalid instruction";
        case VM_C_ERROR_MEMORY_BOUNDS_ERROR: return "Memory bounds error";
        case VM_C_ERROR_IO_ERROR:            return "I/O error";
        case VM_C_ERROR_PROGRAM_NOT_LOADED:  return "Program not loaded";
        default:                             return "Unknown error";
    }
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

// === Legacy Compatibility Functions ===

int vm_init_compat(ComponentVM_C** vm_ptr) {
    if (!vm_ptr) {
        return 1; // Error
    }
    
    *vm_ptr = component_vm_create();
    return (*vm_ptr) ? 0 : 1;
}

int vm_load_program_compat(ComponentVM_C* vm, uint16_t* program, uint32_t size) {
    if (!vm || !program) {
        return 1; // Error
    }
    
    // Convert legacy 16-bit instructions to new 32-bit format
    vm_instruction_c_t* converted_program = new(std::nothrow) vm_instruction_c_t[size];
    if (!converted_program) {
        return 1; // Memory allocation error
    }
    
    for (uint32_t i = 0; i < size; i++) {
        uint16_t legacy_instr = program[i];
        // Legacy format: opcode in upper 8 bits, immediate in lower 8 bits
        converted_program[i].opcode = (uint8_t)((legacy_instr >> 8) & 0xFF);
        converted_program[i].flags = 0;
        converted_program[i].immediate = (uint16_t)(legacy_instr & 0xFF);
    }
    
    bool result = component_vm_load_program(vm, converted_program, size);
    
    delete[] converted_program;
    return result ? 0 : 1;
}

int vm_run_compat(ComponentVM_C* vm, uint32_t max_cycles) {
    // max_cycles is ignored in new implementation - it runs until halt or error
    if (!vm) {
        return 1; // Error
    }
    
    // Execute single steps until halted or error
    while (!component_vm_is_halted(vm)) {
        if (!component_vm_execute_single_step(vm)) {
            return 1; // Execution error
        }
    }
    
    return 0; // Success
}
