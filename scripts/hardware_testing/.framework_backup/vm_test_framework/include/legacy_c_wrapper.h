/*
 * Legacy C Test Wrapper
 * Phase 4.3.2B: Gradual migration support for existing C tests
 * 
 * Wraps existing C test functions in C++ test framework
 * Maintains compatibility while transitioning to native C++ tests
 */

#pragma once

#include "vm_test_base.h"

// Simple wrapper data - just function pointer and name
struct LegacyCTestData {
    const char* test_name;
    void (*test_function)(void);
    uint32_t expected_timeout_ms;
    
    // Default constructor for template compatibility
    LegacyCTestData() : test_name("unknown"), test_function(nullptr), expected_timeout_ms(10000) {}
    
    LegacyCTestData(const char* name, void (*func)(void), uint32_t timeout = 10000)
        : test_name(name), test_function(func), expected_timeout_ms(timeout) {}
};

class LegacyCTestWrapper : public VMTestBase<LegacyCTestData> {
private:
    bool execution_completed;
    uint32_t execution_start_time;
    
public:
    LegacyCTestWrapper(const char* test_name) : VMTestBase(std::string("Legacy_") + test_name) {
        enable_observer(false);  // C tests don't use observers directly
        execution_completed = false;
    }

protected:
    void setup_test_specific() override {
        debug_test_print("Setting up legacy C test wrapper");
        execution_completed = false;
        execution_start_time = get_current_time_ms();
    }
    
    bool execute_test_logic() override {
        if (!test_data.test_function) {
            result.error_message = "No test function provided";
            return false;
        }
        
        debug_test_print("Executing legacy C test: " + std::string(test_data.test_name));
        
        // Execute the C test function
        // Note: C tests are responsible for their own pass/fail determination
        // Exception handling disabled in embedded build
        test_data.test_function();
        execution_completed = true;
        
        debug_test_print("Legacy C test completed");
        return true;
    }
    
    void validate_results(const LegacyCTestData& expected) override {
        debug_test_print("Validating legacy C test results");
        
        // For legacy tests, we primarily validate:
        // 1. Test completed without crashing
        // 2. Test completed within timeout
        // 3. Test function was actually called
        
        if (!execution_completed) {
            result.error_message = "Legacy C test did not complete";
            return;
        }
        
        uint32_t execution_time = get_current_time_ms() - execution_start_time;
        if (execution_time > expected.expected_timeout_ms) {
            result.error_message = "Legacy C test exceeded timeout. Expected: " +
                                  std::to_string(expected.expected_timeout_ms) + "ms, Actual: " +
                                  std::to_string(execution_time) + "ms";
            return;
        }
        
        debug_test_print("✓ Legacy C test validation passed");
        debug_test_print("Execution time: " + std::to_string(execution_time) + "ms");
    }
    
    void cleanup_test() override {
        debug_test_print("Cleaning up legacy C test wrapper");
        // Most C tests handle their own cleanup, but we can add common cleanup here
    }
};

// Convenience function to create and run legacy C test
inline TestResult run_legacy_c_test(const char* test_name, void (*test_function)(void), uint32_t timeout_ms = 10000) {
    LegacyCTestWrapper wrapper(test_name);
    LegacyCTestData data(test_name, test_function, timeout_ms);
    return wrapper.run_test(data);
}