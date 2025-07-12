/*
 * C++ Native Test Runner
 * Phase 4.3.2B: Main test execution framework
 * 
 * Manages test registration, execution, and reporting
 * Supports both native C++ tests and legacy C test wrappers
 */

#pragma once

#include "vm_test_base.h"
#include "gpio_register_test.h"
#include "arduino_api_test.h"
#include "sos_timing_test.h"
#include "legacy_c_wrapper.h"
#include <vector>
#include <memory>
#include <functional>

// Test suite results
struct TestSuiteResult {
    size_t total_tests;
    size_t passed_tests;
    size_t failed_tests;
    std::vector<TestResult> individual_results;
    uint32_t total_execution_time_ms;
    
    TestSuiteResult() : total_tests(0), passed_tests(0), failed_tests(0), total_execution_time_ms(0) {}
    
    double get_success_rate() const {
        return total_tests > 0 ? (double)passed_tests / total_tests * 100.0 : 0.0;
    }
};

// Base class for test factories to enable polymorphism
class TestFactory {
public:
    virtual ~TestFactory() = default;
    virtual TestResult run_test() = 0;
    virtual std::string get_test_name() const = 0;
};

// Template factory for type-safe test creation
template<typename TestClass, typename TestDataType>
class TypedTestFactory : public TestFactory {
private:
    TestDataType test_data;
    std::string test_name;
    
public:
    TypedTestFactory(const std::string& name, const TestDataType& data) 
        : test_name(name), test_data(data) {}
    
    TestResult run_test() override {
        TestClass test;
        return test.run_test(test_data);
    }
    
    std::string get_test_name() const override {
        return test_name;
    }
};

// Main test runner class
class CppTestRunner {
private:
    std::vector<std::unique_ptr<TestFactory>> test_factories;
    TestSuiteResult last_result;
    
public:
    CppTestRunner() = default;
    
    // Register native C++ tests
    template<typename TestClass, typename TestDataType>
    void register_test(const std::string& test_name, const TestDataType& test_data) {
        auto factory = std::make_unique<TypedTestFactory<TestClass, TestDataType>>(test_name, test_data);
        test_factories.push_back(std::move(factory));
    }
    
    // Register legacy C tests
    void register_legacy_test(const char* test_name, void (*test_function)(void), uint32_t timeout_ms = 10000) {
        LegacyCTestData data(test_name, test_function, timeout_ms);
        register_test<LegacyCTestWrapper>("Legacy_" + std::string(test_name), data);
    }
    
    // Run all registered tests
    TestSuiteResult run_all_tests() {
        last_result = TestSuiteResult();
        
#ifdef HARDWARE_PLATFORM
        debug_print("=== C++ NATIVE TEST SUITE START ===");
        debug_print(("Total tests to run: " + std::to_string(test_factories.size())).c_str());
#endif
        
        uint32_t suite_start_time = get_current_time_ms();
        
        for (auto& factory : test_factories) {
            std::string test_name = factory->get_test_name();
            
#ifdef HARDWARE_PLATFORM
            debug_print(("Running test: " + test_name).c_str());
#endif
            
            TestResult result = factory->run_test();
            last_result.individual_results.push_back(result);
            last_result.total_tests++;
            
            if (result.passed) {
                last_result.passed_tests++;
#ifdef HARDWARE_PLATFORM
                debug_print(("✓ " + test_name + ": PASSED").c_str());
#endif
            } else {
                last_result.failed_tests++;
#ifdef HARDWARE_PLATFORM
                debug_print(("✗ " + test_name + ": FAILED - " + result.error_message).c_str());
#endif
            }
        }
        
        uint32_t suite_end_time = get_current_time_ms();
        last_result.total_execution_time_ms = suite_end_time - suite_start_time;
        
#ifdef HARDWARE_PLATFORM
        debug_print("=== C++ NATIVE TEST SUITE COMPLETE ===");
        debug_print(("Tests passed: " + std::to_string(last_result.passed_tests) + 
                    "/" + std::to_string(last_result.total_tests)).c_str());
        debug_print(("Success rate: " + std::to_string((int)last_result.get_success_rate()) + "%").c_str());
        debug_print(("Total execution time: " + std::to_string(last_result.total_execution_time_ms) + "ms").c_str());
#endif
        
        return last_result;
    }
    
    // Run specific test by name
    TestResult run_test_by_name(const std::string& name) {
        for (auto& factory : test_factories) {
            if (factory->get_test_name() == name) {
#ifdef HARDWARE_PLATFORM
                debug_print(("Running specific test: " + name).c_str());
#endif
                return factory->run_test();
            }
        }
        
        TestResult not_found(name);
        not_found.error_message = "Test not found: " + name;
        return not_found;
    }
    
    // Get last test suite results
    const TestSuiteResult& get_last_results() const {
        return last_result;
    }
    
    // Utility function
    uint32_t get_current_time_ms() const {
#ifdef HARDWARE_PLATFORM
        return HAL_GetTick();
#else
        return 0;
#endif
    }
};