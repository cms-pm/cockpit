#pragma once

#include "bytecode_visitor.h"
#include "components/component_vm.h"
#include <vector>
#include <memory>

class VMIntegration {
public:
    VMIntegration();
    ~VMIntegration();
    
    // Convert compiler bytecode to ComponentVM instructions
    bool load_program_from_bytecode(const std::vector<Instruction>& compiler_bytecode);
    
    // Execute the loaded program
    bool execute_program();
    
    // Single-step execution for debugging
    bool execute_single_step();
    
    // VM state management
    void reset_vm();
    bool is_running() const;
    bool is_halted() const;
    
    // Access to VM components for debugging
    const ComponentVM& get_vm() const { return *vm_; }
    ComponentVM& get_vm() { return *vm_; }
    
    // Performance and error reporting
    ComponentVM::PerformanceMetrics get_performance_metrics() const;
    ComponentVM::VMError get_last_error() const;
    const char* get_error_string() const;
    
private:
    std::unique_ptr<ComponentVM> vm_;
    std::vector<VMInstruction> component_instructions_;  // ComponentVM format
    
    // Convert compiler instruction to ComponentVM instruction
    bool convert_instruction(const Instruction& compiler_instr, VMInstruction& vm_instr);
    
    // Validate instruction conversion
    bool validate_program(const std::vector<VMInstruction>& instructions);
};