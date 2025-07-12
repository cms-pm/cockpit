/*
 * SOS Timing Validation Test
 * Phase 4.3.2B: SOS pattern timing validation for SOS program
 * 
 * Validates delay() timing accuracy and SOS pattern timing requirements
 * Tests short blinks (200ms), long blinks (600ms), and gaps (200ms)
 */

#pragma once

#include "vm_test_base.h"
#include "../../component_vm/include/component_vm.h"

// Test data structure for SOS timing validation
struct SOSTimingTestData {
    struct TimingExpectation {
        uint32_t delay_ms;
        uint32_t tolerance_ms;    // Acceptable timing variance
        size_t expected_count;    // How many times this delay should occur
    };
    
    std::vector<TimingExpectation> expected_delays;
    std::vector<VM::Instruction> sos_program;
    uint32_t total_pattern_time_ms;
    uint32_t pattern_tolerance_ms;
    
    SOSTimingTestData() : total_pattern_time_ms(0), pattern_tolerance_ms(100) {}
};

class SOSTimingTest : public VMTestBase<SOSTimingTestData> {
private:
    std::vector<uint32_t> observed_delays;
    std::vector<uint32_t> delay_timestamps;
    uint32_t pattern_start_time;
    uint32_t pattern_end_time;
    
public:
    SOSTimingTest() : VMTestBase("SOS_Timing_Validation") {
        enable_observer(true);  // Monitor for delay instructions
    }

protected:
    void setup_test_specific() override {
        debug_test_print("Starting SOS timing validation");
        observed_delays.clear();
        delay_timestamps.clear();
        pattern_start_time = get_current_time_ms();
    }
    
    bool execute_test_logic() override {
        if (test_data.sos_program.empty()) {
            result.error_message = "No SOS timing test program provided";
            return false;
        }
        
        debug_test_print("Executing SOS timing test program");
        pattern_start_time = get_current_time_ms();
        
        bool success = vm.execute_program(test_data.sos_program.data(),
                                        test_data.sos_program.size());
        
        pattern_end_time = get_current_time_ms();
        
        if (!success) {
            result.error_message = "Failed to execute SOS timing program: " +
                                  std::string(vm.get_error_string(vm.get_last_error()));
            return false;
        }
        
        debug_test_print("SOS timing program executed successfully");
        return true;
    }
    
    void validate_results(const SOSTimingTestData& expected) override {
        debug_test_print("Validating SOS timing patterns");
        
        // Validate total pattern execution time
        uint32_t actual_total_time = pattern_end_time - pattern_start_time;
        if (!validate_total_timing(actual_total_time, expected)) {
            return; // Error message set by validate_total_timing
        }
        
        // Validate individual delay timings
        if (!validate_delay_patterns(expected.expected_delays)) {
            return; // Error message set by validate_delay_patterns  
        }
        
        debug_test_print("✓ SOS timing validation passed");
        debug_test_print("Total pattern time: " + std::to_string(actual_total_time) + "ms");
        debug_test_print("Delays observed: " + std::to_string(observed_delays.size()));
    }

private:
    bool validate_total_timing(uint32_t actual_time, const SOSTimingTestData& expected) {
        if (expected.total_pattern_time_ms == 0) {
            return true; // No total time expectation
        }
        
        uint32_t min_time = expected.total_pattern_time_ms - expected.pattern_tolerance_ms;
        uint32_t max_time = expected.total_pattern_time_ms + expected.pattern_tolerance_ms;
        
        if (actual_time < min_time || actual_time > max_time) {
            result.error_message = "Total pattern timing out of range. Expected: " +
                                  std::to_string(expected.total_pattern_time_ms) + "±" +
                                  std::to_string(expected.pattern_tolerance_ms) + "ms, Actual: " +
                                  std::to_string(actual_time) + "ms";
            return false;
        }
        
        return true;
    }
    
    bool validate_delay_patterns(const std::vector<SOSTimingTestData::TimingExpectation>& expected_delays) {
        // Count occurrences of each expected delay
        for (const auto& expectation : expected_delays) {
            size_t count = 0;
            
            for (uint32_t observed_delay : observed_delays) {
                uint32_t min_delay = expectation.delay_ms - expectation.tolerance_ms;
                uint32_t max_delay = expectation.delay_ms + expectation.tolerance_ms;
                
                if (observed_delay >= min_delay && observed_delay <= max_delay) {
                    count++;
                }
            }
            
            if (count != expectation.expected_count) {
                result.error_message = "Delay pattern mismatch for " + 
                                      std::to_string(expectation.delay_ms) + "ms delays. Expected: " +
                                      std::to_string(expectation.expected_count) + ", Actual: " +
                                      std::to_string(count);
                return false;
            }
        }
        
        return true;
    }

public:
    // Override observer to detect delay instructions and measure timing
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        VMTestBase::on_instruction_executed(pc, opcode, operand);
        
        // Detect delay instructions
        if (opcode == 0x14) {  // Assuming OP_DELAY = 0x14
            uint32_t delay_value = operand;
            observed_delays.push_back(delay_value);
            delay_timestamps.push_back(get_current_time_ms());
            
            debug_test_print("Detected delay(" + std::to_string(delay_value) + "ms) at PC " + 
                           std::to_string(pc));
        }
    }
};