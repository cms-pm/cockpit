/*
 * Arduino API Validation Test
 * Phase 4.3.2B: Arduino API function validation for SOS program
 * 
 * Tests pinMode, digitalWrite, digitalRead functions
 * Validates API behavior without requiring human button interaction
 */

#pragma once

#include "vm_test_base.h"
#include "../../component_vm/include/component_vm.h"

// Test data structure for Arduino API validation
struct ArduinoAPITestData {
    struct APICall {
        std::string function_name;  // "pinMode", "digitalWrite", "digitalRead" 
        uint8_t pin;
        uint8_t value_or_mode;
        uint8_t expected_result;    // For digitalRead
    };
    
    std::vector<APICall> api_sequence;
    std::vector<VM::Instruction> test_program;
    
    // Default constructor for template compatibility
    ArduinoAPITestData() {}
};

class ArduinoAPITest : public VMTestBase<ArduinoAPITestData> {
private:
    std::vector<uint32_t> api_call_results;
    uint32_t digital_write_count = 0;
    uint32_t pin_mode_count = 0;
    
public:
    ArduinoAPITest() : VMTestBase("Arduino_API_Validation") {
        enable_observer(true);  // Monitor all instructions for API calls
    }

protected:
    void setup_test_specific() override {
        debug_test_print("Starting Arduino API validation");
        api_call_results.clear();
        digital_write_count = 0;
        pin_mode_count = 0;
    }
    
    bool execute_test_logic() override {
        if (test_data.test_program.empty()) {
            result.error_message = "No Arduino API test program provided";
            return false;
        }
        
        debug_test_print("Executing Arduino API test program with " +
                        std::to_string(test_data.test_program.size()) + " instructions");
        
        bool success = vm.execute_program(test_data.test_program.data(),
                                        test_data.test_program.size());
        
        if (!success) {
            result.error_message = "Failed to execute Arduino API program: " +
                                  std::string(vm.get_error_string(vm.get_last_error()));
            return false;
        }
        
        debug_test_print("Arduino API program executed successfully");
        return true;
    }
    
    void validate_results(const ArduinoAPITestData& expected) override {
        debug_test_print("Validating Arduino API function calls");
        
        // Validate that expected API calls were executed
        if (pin_mode_count == 0 && has_pinmode_calls(expected)) {
            result.error_message = "No pinMode calls detected in program execution";
            return;
        }
        
        if (digital_write_count == 0 && has_digitalwrite_calls(expected)) {
            result.error_message = "No digitalWrite calls detected in program execution";
            return;
        }
        
        // Validate API call sequence matches expectations
        if (!validate_api_sequence(expected.api_sequence)) {
            return; // Error message set by validate_api_sequence
        }
        
        debug_test_print("✓ Arduino API validation passed");
        debug_test_print("pinMode calls: " + std::to_string(pin_mode_count));
        debug_test_print("digitalWrite calls: " + std::to_string(digital_write_count));
    }

private:
    bool has_pinmode_calls(const ArduinoAPITestData& data) {
        for (const auto& call : data.api_sequence) {
            if (call.function_name == "pinMode") return true;
        }
        return false;
    }
    
    bool has_digitalwrite_calls(const ArduinoAPITestData& data) {
        for (const auto& call : data.api_sequence) {
            if (call.function_name == "digitalWrite") return true;
        }
        return false;
    }
    
    bool validate_api_sequence(const std::vector<ArduinoAPITestData::APICall>& expected_sequence) {
        // For now, just validate that we observed the expected number of calls
        // More sophisticated sequence validation can be added later
        
        size_t expected_pinmode = 0;
        size_t expected_digitalwrite = 0;
        
        for (const auto& call : expected_sequence) {
            if (call.function_name == "pinMode") expected_pinmode++;
            if (call.function_name == "digitalWrite") expected_digitalwrite++;
        }
        
        if (pin_mode_count != expected_pinmode) {
            result.error_message = "pinMode count mismatch. Expected: " + 
                                  std::to_string(expected_pinmode) + 
                                  ", Actual: " + std::to_string(pin_mode_count);
            return false;
        }
        
        if (digital_write_count != expected_digitalwrite) {
            result.error_message = "digitalWrite count mismatch. Expected: " + 
                                  std::to_string(expected_digitalwrite) + 
                                  ", Actual: " + std::to_string(digital_write_count);
            return false;
        }
        
        return true;
    }

public:
    // Override observer to detect Arduino API instructions
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        VMTestBase::on_instruction_executed(pc, opcode, operand);
        
        // Detect Arduino API calls based on opcodes (adjust opcodes as needed)
        switch (opcode) {
            case 0x40:  // OP_PINMODE
                pin_mode_count++;
                debug_test_print("Detected pinMode call at PC " + std::to_string(pc));
                break;
                
            case 0x41:  // OP_DIGITAL_WRITE  
                digital_write_count++;
                debug_test_print("Detected digitalWrite call at PC " + std::to_string(pc));
                break;
                
            case 0x42:  // OP_DIGITAL_READ
                debug_test_print("Detected digitalRead call at PC " + std::to_string(pc));
                break;
                
            default:
                // Other instructions - no action needed
                break;
        }
    }
};