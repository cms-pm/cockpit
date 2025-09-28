#include "vm_integration.h"
#include <iostream>
#include <cstring>

// Include for vm_compiler compatibility
#include "bytecode_visitor.h"

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

vm_error_t VMIntegration::get_last_error() const {
    return vm_->get_last_error();
}

const char* VMIntegration::get_error_string() const {
    return vm_->get_error_string(vm_->get_last_error());
}

bool VMIntegration::convert_instruction(const Instruction& compiler_instr, VM::Instruction& vm_instr) {
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

bool VMIntegration::validate_program(const std::vector<VM::Instruction>& instructions) {
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

// GT Lite specific methods
bool VMIntegration::load_vm_instructions(const VM::Instruction* instructions, size_t count) {
    if (!instructions || count == 0) {
        std::cerr << "Invalid instructions or count" << std::endl;
        return false;
    }

    component_instructions_.clear();
    component_instructions_.reserve(count);

    // Copy VM::Instructions directly (no conversion needed)
    for (size_t i = 0; i < count; i++) {
        component_instructions_.push_back(instructions[i]);
    }

    // Validate the program
    if (!validate_program(component_instructions_)) {
        std::cerr << "Program validation failed" << std::endl;
        return false;
    }

    // Load program into ComponentVM
    return vm_->load_program(component_instructions_.data(), component_instructions_.size());
}

bool VMIntegration::load_bytecode_array(const uint8_t* bytecode, size_t size) {
    if (!bytecode || size == 0) {
        std::cerr << "Invalid bytecode or size" << std::endl;
        return false;
    }

    return convert_bytecode_to_instructions(bytecode, size);
}

bool VMIntegration::execute_program_with_timeout(uint32_t timeout_ms) {
    if (component_instructions_.empty()) {
        std::cerr << "No program loaded" << std::endl;
        return false;
    }

    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(timeout_ms);

    // Reset VM state before execution
    vm_->reset_vm();

    while (!vm_->is_halted()) {
        if (!vm_->execute_single_step()) {
            return false;
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > timeout) {
            std::cerr << "Execution timeout after " << timeout_ms << "ms" << std::endl;
            return false;
        }
    }

    return true;
}

int32_t VMIntegration::get_stack_top() const {
    // Access ComponentVM stack through execution engine
    const auto& execution_engine = vm_->get_execution_engine();
    return execution_engine.get_stack_top();
}

size_t VMIntegration::get_stack_size() const {
    const auto& execution_engine = vm_->get_execution_engine();
    return execution_engine.get_stack_size();
}

const int32_t* VMIntegration::get_stack_contents() const {
    const auto& execution_engine = vm_->get_execution_engine();
    return execution_engine.get_stack_contents();
}

bool VMIntegration::convert_bytecode_to_instructions(const uint8_t* bytecode, size_t size) {
    // Ensure size is multiple of VM::Instruction size
    if (size % sizeof(VM::Instruction) != 0) {
        std::cerr << "Bytecode size not aligned to VM::Instruction size" << std::endl;
        return false;
    }

    size_t instruction_count = size / sizeof(VM::Instruction);
    const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(bytecode);

    return load_vm_instructions(instructions, instruction_count);
}