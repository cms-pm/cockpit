#include "vm_integration.h"
#include <iostream>

VMIntegration::VMIntegration() : vm_(std::make_unique<ComponentVM>()) {
}

VMIntegration::~VMIntegration() = default;

bool VMIntegration::load_program_from_bytecode(const std::vector<Instruction>& compiler_bytecode) {
    component_instructions_.clear();
    component_instructions_.reserve(compiler_bytecode.size());
    
    // Convert each compiler instruction to ComponentVM format
    for (const auto& compiler_instr : compiler_bytecode) {
        VMInstruction vm_instr{};
        if (!convert_instruction(compiler_instr, vm_instr)) {
            std::cerr << "Failed to convert instruction: opcode=" 
                      << static_cast<int>(compiler_instr.opcode) << std::endl;
            return false;
        }
        component_instructions_.push_back(vm_instr);
    }
    
    // Validate the converted program
    if (!validate_program(component_instructions_)) {
        std::cerr << "Program validation failed" << std::endl;
        return false;
    }
    
    // Load program into ComponentVM
    return vm_->load_program(component_instructions_.data(), component_instructions_.size());
}

bool VMIntegration::execute_program() {
    if (component_instructions_.empty()) {
        std::cerr << "No program loaded" << std::endl;
        return false;
    }
    
    return vm_->execute_program(component_instructions_.data(), component_instructions_.size());
}

bool VMIntegration::execute_single_step() {
    return vm_->execute_single_step();
}

void VMIntegration::reset_vm() {
    vm_->reset_vm();
}

bool VMIntegration::is_running() const {
    return vm_->is_running();
}

bool VMIntegration::is_halted() const {
    return vm_->is_halted();
}

ComponentVM::PerformanceMetrics VMIntegration::get_performance_metrics() const {
    return vm_->get_performance_metrics();
}

ComponentVM::VMError VMIntegration::get_last_error() const {
    return vm_->get_last_error();
}

const char* VMIntegration::get_error_string() const {
    return vm_->get_error_string(vm_->get_last_error());
}

bool VMIntegration::convert_instruction(const Instruction& compiler_instr, VMInstruction& vm_instr) {
    // Convert VMOpcode enum to uint8_t opcode
    vm_instr.opcode = static_cast<uint8_t>(compiler_instr.opcode);
    vm_instr.flags = compiler_instr.flags;
    vm_instr.immediate = compiler_instr.immediate;
    
    // Validate opcode range
    if (vm_instr.opcode == 0 || vm_instr.opcode > 0x6F) {
        // Allow HALT (0x0A) but reject invalid opcodes
        if (vm_instr.opcode != 0x0A) {
            std::cerr << "Invalid opcode: 0x" << std::hex 
                      << static_cast<int>(vm_instr.opcode) << std::dec << std::endl;
            return false;
        }
    }
    
    return true;
}

bool VMIntegration::validate_program(const std::vector<VMInstruction>& instructions) {
    if (instructions.empty()) {
        std::cerr << "Empty program" << std::endl;
        return false;
    }
    
    // Check for proper program termination
    bool has_halt = false;
    for (const auto& instr : instructions) {
        if (instr.opcode == 0x0A) {  // OP_HALT
            has_halt = true;
            break;
        }
    }
    
    if (!has_halt) {
        std::cerr << "Warning: Program does not contain HALT instruction" << std::endl;
        // Don't fail validation, just warn
    }
    
    // Additional validation could be added here:
    // - Jump target validation
    // - Stack balance analysis
    // - Resource usage analysis
    
    return true;
}