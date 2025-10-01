#pragma once

#include "component_vm.h"
#include <vector>
#include <memory>
#include <chrono>

// Forward declarations for vm_compiler compatibility
struct Instruction;

class VMIntegration {
public:
    VMIntegration();
    ~VMIntegration();

    // GT Lite interface - direct VM::Instruction loading
    bool load_vm_instructions(const VM::Instruction* instructions, size_t count);
    bool load_bytecode_array(const uint8_t* bytecode, size_t size);

    // vm_compiler compatibility interface
    bool load_program_from_bytecode(const std::vector<Instruction>& compiler_bytecode);

    // Execution with timeout protection
    bool execute_program();
    bool execute_program_with_timeout(uint32_t timeout_ms = 10000);
    bool execute_single_step();

    // VM state management
    void reset_vm();
    bool is_running() const;
    bool is_halted() const;

    // Access to VM components for debugging
    const ComponentVM& get_vm() const { return *vm_; }
    ComponentVM& get_vm() { return *vm_; }

    // GT Lite specific validation access
    int32_t get_stack_top() const;
    size_t get_stack_size() const;
    const int32_t* get_stack_contents() const;

    // Performance and error reporting
    ComponentVM::PerformanceMetrics get_performance_metrics() const;
    vm_error_t get_last_error() const;
    const char* get_error_string() const;

private:
    std::unique_ptr<ComponentVM> vm_;
    std::vector<VM::Instruction> component_instructions_;  // ComponentVM format

    // Convert compiler instruction to ComponentVM instruction
    bool convert_instruction(const Instruction& compiler_instr, VM::Instruction& vm_instr);

    // Validate instruction conversion
    bool validate_program(const std::vector<VM::Instruction>& instructions);

    // Convert bytecode array to VM::Instruction format
    bool convert_bytecode_to_instructions(const uint8_t* bytecode, size_t size);
};