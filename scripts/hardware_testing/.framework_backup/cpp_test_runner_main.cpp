/*
 * C++ Native Test Runner - Main Implementation
 * Phase 4.3.2B: Complete SOS hardware validation test suite
 * 
 * Demonstrates C++ test framework with SOS-focused tests
 * Shows direct ComponentVM usage with observer pattern
 */

#include <cstdint>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    #include "../lib/semihosting/semihosting.h"
    #include "../lib/vm_test_framework/include/cpp_test_runner.h"
    #include "../lib/component_vm/include/component_vm.h"
#endif

#ifdef HARDWARE_PLATFORM

// Forward declarations for legacy C tests
extern "C" {
    void run_telemetry_validation_main(void);
    void run_observer_pattern_test_main(void);
}

// SOS program opcodes (from ComponentVM instruction set)
constexpr uint8_t OP_HALT = 0x00;
constexpr uint8_t OP_PUSH = 0x01;
constexpr uint8_t OP_ADD = 0x03;
constexpr uint8_t OP_DELAY = 0x14;
constexpr uint8_t OP_PINMODE = 0x40;
constexpr uint8_t OP_DIGITAL_WRITE = 0x41;
constexpr uint8_t OP_DIGITAL_READ = 0x42;

// Arduino API constants
constexpr uint8_t INPUT = 0;
constexpr uint8_t OUTPUT = 1;
constexpr uint8_t INPUT_PULLUP = 2;
constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;

void setup_sos_hardware_tests(CppTestRunner& runner) {
    debug_print("Setting up SOS hardware validation tests");
    
    // Test 1: GPIO Register Validation - PC6 OUTPUT
    {
        GPIORegisterTestData pc6_output_test(6, STM32G4_Registers::OUTPUT_MODE, 0);
        // pinMode(6, OUTPUT) program
        pc6_output_test.setup_program = {
            {OP_PUSH, 0, 6},        // pin 6
            {OP_PUSH, 0, OUTPUT},   // OUTPUT mode
            {OP_PINMODE, 0, 0},     // pinMode(6, OUTPUT)
            {OP_HALT, 0, 0}
        };
        runner.register_test<GPIORegisterTest>("PC6_OUTPUT_Register_Validation", pc6_output_test);
    }
    
    // Test 2: GPIO Register Validation - PC13 INPUT_PULLUP  
    {
        GPIORegisterTestData pc13_input_test(13, STM32G4_Registers::INPUT_MODE, STM32G4_Registers::PULLUP_MODE);
        // pinMode(13, INPUT_PULLUP) program
        pc13_input_test.setup_program = {
            {OP_PUSH, 0, 13},           // pin 13
            {OP_PUSH, 0, INPUT_PULLUP}, // INPUT_PULLUP mode
            {OP_PINMODE, 0, 0},         // pinMode(13, INPUT_PULLUP)
            {OP_HALT, 0, 0}
        };
        runner.register_test<GPIORegisterTest>("PC13_INPUT_PULLUP_Register_Validation", pc13_input_test);
    }
    
    // Test 3: Arduino API - digitalWrite Sequence
    {
        ArduinoAPITestData api_test;
        api_test.api_sequence = {
            {"pinMode", 6, OUTPUT, 0},
            {"digitalWrite", 6, HIGH, 0},
            {"digitalWrite", 6, LOW, 0}
        };
        // Complete Arduino API test program
        api_test.test_program = {
            {OP_PUSH, 0, 6},         // pin 6
            {OP_PUSH, 0, OUTPUT},    // OUTPUT mode
            {OP_PINMODE, 0, 0},      // pinMode(6, OUTPUT)
            {OP_PUSH, 0, 6},         // pin 6
            {OP_PUSH, 0, HIGH},      // HIGH value
            {OP_DIGITAL_WRITE, 0, 0}, // digitalWrite(6, HIGH)
            {OP_PUSH, 0, 6},         // pin 6
            {OP_PUSH, 0, LOW},       // LOW value
            {OP_DIGITAL_WRITE, 0, 0}, // digitalWrite(6, LOW)
            {OP_HALT, 0, 0}
        };
        runner.register_test<ArduinoAPITest>("Arduino_API_digitalWrite_Validation", api_test);
    }
    
    // Test 4: SOS Timing Pattern Validation
    {
        SOSTimingTestData sos_timing;
        sos_timing.expected_delays = {
            {200, 50, 6},   // Short blinks: 200ms ± 50ms, 6 times (S-O-S pattern)
            {600, 100, 3},  // Long blinks: 600ms ± 100ms, 3 times (O pattern)
            {1000, 200, 1}  // Final gap: 1000ms ± 200ms, 1 time
        };
        sos_timing.total_pattern_time_ms = 5000;  // Approximate total SOS pattern time
        sos_timing.pattern_tolerance_ms = 500;    // ± 500ms tolerance
        
        // Simple SOS pattern program (S-O-S)
        sos_timing.sos_program = {
            // S: 3 short blinks
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF (gap)
            
            // O: 3 long blinks  
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 600}, {OP_DELAY, 0, 0},  // 600ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 600}, {OP_DELAY, 0, 0},  // 600ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 600}, {OP_DELAY, 0, 0},  // 600ms ON
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},  // 200ms OFF (gap)
            
            // S: 3 short blinks (repeat of first S)
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},
            
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, HIGH}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 200}, {OP_DELAY, 0, 0},
            {OP_PUSH, 0, 6}, {OP_PUSH, 0, LOW}, {OP_DIGITAL_WRITE, 0, 0},
            {OP_PUSH, 0, 1000}, {OP_DELAY, 0, 0}, // Final 1000ms gap
            
            {OP_HALT, 0, 0}
        };
        runner.register_test<SOSTimingTest>("SOS_Pattern_Timing_Validation", sos_timing);
    }
    
    debug_print("SOS hardware validation tests configured");
}

void run_cpp_native_test_suite(void) {
    debug_print("=== C++ NATIVE TEST FRAMEWORK DEMONSTRATION ===");
    debug_print("Phase 4.3.2B: Direct ComponentVM usage with observer pattern");
    debug_print("SOS hardware validation: PC13 button + PC6 LED");
    debug_print("");
    
    CppTestRunner runner;
    
    // Setup SOS-focused hardware validation tests
    setup_sos_hardware_tests(runner);
    
    // Register legacy C tests for compatibility
    runner.register_legacy_test("telemetry_validation", run_telemetry_validation_main, 15000);
    runner.register_legacy_test("observer_pattern_basic", run_observer_pattern_test_main, 10000);
    
    // Run complete test suite
    debug_print("Starting C++ native test suite execution...");
    TestSuiteResult results = runner.run_all_tests();
    
    // Report final results
    debug_print("=== FINAL TEST SUITE RESULTS ===");
    debug_print(("Total tests: " + std::to_string(results.total_tests)).c_str());
    debug_print(("Passed: " + std::to_string(results.passed_tests)).c_str());
    debug_print(("Failed: " + std::to_string(results.failed_tests)).c_str());
    debug_print(("Success rate: " + std::to_string((int)results.get_success_rate()) + "%").c_str());
    debug_print(("Total execution time: " + std::to_string(results.total_execution_time_ms) + "ms").c_str());
    
    // Success indicator - ultra-fast blink for C++ test completion
    debug_print("C++ native test suite complete - entering ultra-fast blink mode");
    
    while(1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);    // LED ON
        HAL_Delay(50);    // 50ms ON (ultra-fast blink)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED OFF
        HAL_Delay(50);    // 50ms OFF (ultra-fast blink)
        
        // Periodic status
        debug_print("C++ native test framework validation complete");
    }
}

#endif // HARDWARE_PLATFORM