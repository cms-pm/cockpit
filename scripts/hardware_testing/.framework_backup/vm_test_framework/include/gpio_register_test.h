/*
 * GPIO Register Validation Test
 * Phase 4.3.2B: Register-level validation for SOS hardware requirements
 * 
 * Validates PC13 INPUT_PULLUP and PC6 OUTPUT register configuration
 * Tests Arduino API pinMode() effects on actual STM32G431CB registers
 */

#pragma once

#include "vm_test_base.h"
#include "../../component_vm/include/component_vm.h"

// Test data structure for GPIO register validation
struct GPIORegisterTestData {
    uint8_t pin;
    uint8_t expected_mode;        // INPUT=0, OUTPUT=1
    uint8_t expected_pullup;      // NOPULL=0, PULLUP=1, PULLDOWN=2
    std::vector<VM::Instruction> setup_program;  // pinMode bytecode
    
    // Default constructor for template compatibility
    GPIORegisterTestData() : pin(0), expected_mode(0), expected_pullup(0) {}
    
    GPIORegisterTestData(uint8_t p, uint8_t mode, uint8_t pullup) 
        : pin(p), expected_mode(mode), expected_pullup(pullup) {}
};

class GPIORegisterTest : public VMTestBase<GPIORegisterTestData> {
private:
    uint32_t initial_moder_value;
    uint32_t initial_pupdr_value;
    
public:
    GPIORegisterTest() : VMTestBase("GPIO_Register_Validation") {
        enable_observer(true);  // Enable instruction observation
    }

protected:
    void setup_test_specific() override {
        debug_test_print("Starting GPIO register validation");
        
        // Capture initial register state
        initial_moder_value = read_gpio_register(STM32G4_Registers::GPIOC_MODER);
        initial_pupdr_value = read_gpio_register(STM32G4_Registers::GPIOC_PUPDR);
        
        debug_test_print("Initial MODER: 0x" + std::to_string(initial_moder_value));
        debug_test_print("Initial PUPDR: 0x" + std::to_string(initial_pupdr_value));
    }
    
    bool execute_test_logic() override {
        // Execute pinMode bytecode program to configure GPIO
        if (test_data.setup_program.empty()) {
            result.error_message = "No setup program provided";
            return false;
        }
        
        debug_test_print("Executing pinMode program with " + 
                        std::to_string(test_data.setup_program.size()) + " instructions");
        
        bool success = vm.execute_program(test_data.setup_program.data(), 
                                        test_data.setup_program.size());
        
        if (!success) {
            result.error_message = "Failed to execute pinMode program: " + 
                                  std::string(vm.get_error_string(vm.get_last_error()));
            return false;
        }
        
        debug_test_print("pinMode program executed successfully");
        return true;
    }
    
    void validate_results(const GPIORegisterTestData& expected) override {
        debug_test_print("Validating GPIO register configuration");
        
        // Validate pin mode configuration
        if (!validate_pin_mode_bits(expected.pin, expected.expected_mode)) {
            return; // Error message set by validate_pin_mode_bits
        }
        
        // Validate pullup configuration  
        if (!validate_pin_pullup_bits(expected.pin, expected.expected_pullup)) {
            return; // Error message set by validate_pin_pullup_bits
        }
        
        // Additional validation: check that registers actually changed
        uint32_t final_moder = read_gpio_register(STM32G4_Registers::GPIOC_MODER);
        uint32_t final_pupdr = read_gpio_register(STM32G4_Registers::GPIOC_PUPDR);
        
        debug_test_print("Final MODER: 0x" + std::to_string(final_moder));
        debug_test_print("Final PUPDR: 0x" + std::to_string(final_pupdr));
        
        // Verify registers changed (unless already in correct state)
        uint32_t pin_mask = 0x3 << (expected.pin * 2);
        uint32_t expected_moder_bits = expected.expected_mode << (expected.pin * 2);
        uint32_t expected_pupdr_bits = expected.expected_pullup << (expected.pin * 2);
        
        if ((final_moder & pin_mask) != expected_moder_bits) {
            result.error_message = "MODER register not set correctly for pin " + std::to_string(expected.pin);
            return;
        }
        
        if ((final_pupdr & pin_mask) != expected_pupdr_bits) {
            result.error_message = "PUPDR register not set correctly for pin " + std::to_string(expected.pin);
            return;
        }
        
        debug_test_print("✓ GPIO register validation passed");
    }
    
    // Override observer to track pinMode-related instructions
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        VMTestBase::on_instruction_executed(pc, opcode, operand);
        
        // Log pinMode operations (this will be interpreted by specific tests)
        if (opcode == 0x40) {  // Assuming OP_PINMODE = 0x40
            debug_test_print("Observed pinMode instruction at PC " + std::to_string(pc));
        }
    }
};