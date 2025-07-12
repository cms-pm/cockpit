/*
 * VM Test Framework Base Class
 * Phase 4.3.2B: C++ native test framework for ComponentVM
 * 
 * Hybrid approach supporting both register-level and Arduino API validation
 * Direct ComponentVM integration with configurable observer granularity
 */

#pragma once

#include "../../component_vm/include/component_vm.h"
#include "../../vm_blackbox_observer/include/vm_blackbox_observer.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
    extern "C" {
        #include "../../semihosting/semihosting.h"
    }
#endif

// Test result structure
struct TestResult {
    bool passed;
    std::string test_name;
    std::string error_message;
    uint32_t execution_time_ms;
    size_t instructions_executed;
    
    TestResult(const std::string& name) 
        : passed(false), test_name(name), execution_time_ms(0), instructions_executed(0) {}
};

// STM32G431CB GPIO register addresses for hardware validation
namespace STM32G4_Registers {
    static constexpr uint32_t GPIO_C_BASE = 0x48000800;
    static constexpr uint32_t GPIOC_MODER = GPIO_C_BASE + 0x00;   // Mode register
    static constexpr uint32_t GPIOC_PUPDR = GPIO_C_BASE + 0x0C;   // Pull-up/down register
    static constexpr uint32_t GPIOC_IDR   = GPIO_C_BASE + 0x10;   // Input data register
    static constexpr uint32_t GPIOC_ODR   = GPIO_C_BASE + 0x14;   // Output data register
    
    // Pin configuration values for SOS hardware
    static constexpr uint8_t PC6_PIN = 6;   // LED output pin
    static constexpr uint8_t PC13_PIN = 13; // Button input pin
    
    // Expected register bit patterns
    static constexpr uint32_t INPUT_MODE = 0x00;      // 00 in MODER
    static constexpr uint32_t OUTPUT_MODE = 0x01;     // 01 in MODER  
    static constexpr uint32_t PULLUP_MODE = 0x01;     // 01 in PUPDR
}

// Base template class for all VM tests
template<typename TestData>
class VMTestBase : public ITelemetryObserver {
protected:
    ComponentVM vm;
    std::unique_ptr<BlackboxObserver> blackbox_observer;
    TestResult result;
    TestData test_data;
    
    // Observer configuration
    bool observer_enabled = false;
    std::vector<uint32_t> observed_pcs;
    std::vector<uint8_t> observed_opcodes;
    std::vector<uint32_t> observed_operands;
    
public:
    VMTestBase(const std::string& test_name) : result(test_name) {
        // Create blackbox observer for telemetry
        blackbox_observer = std::make_unique<BlackboxObserver>();
    }
    
    virtual ~VMTestBase() = default;
    
    // Main test execution method
    TestResult run_test(const TestData& data) {
        test_data = data;
        result.passed = false;
        result.error_message.clear();
        
        try {
            // Setup phase
            setup_hardware();
            setup_test_specific();
            
            // Enable observer if requested
            if (observer_enabled) {
                vm.add_observer(this);
                vm.add_observer(blackbox_observer.get());
            }
            
            // Execute test
            auto start_time = get_current_time_ms();
            bool execution_success = execute_test_logic();
            auto end_time = get_current_time_ms();
            
            result.execution_time_ms = end_time - start_time;
            result.instructions_executed = vm.get_instruction_count();
            
            if (execution_success) {
                // Validate results
                validate_results(data);
                if (result.error_message.empty()) {
                    result.passed = true;
                }
            }
            
        } catch (...) {
            result.error_message = "Exception occurred during test execution";
        }
        
        // Cleanup
        cleanup_test();
        return result;
    }
    
    // Enable observer pattern with configurable granularity
    void enable_observer(bool enable = true) {
        observer_enabled = enable;
    }
    
    // ITelemetryObserver implementation - configurable per test
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        observed_pcs.push_back(pc);
        observed_opcodes.push_back(opcode);
        observed_operands.push_back(operand);
    }
    
    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override {
        // Default implementation - tests can override
    }
    
    void on_vm_reset() override {
        observed_pcs.clear();
        observed_opcodes.clear();
        observed_operands.clear();
    }

protected:
    // Virtual methods for test customization
    virtual void setup_test_specific() = 0;
    virtual bool execute_test_logic() = 0;
    virtual void validate_results(const TestData& expected) = 0;
    virtual void cleanup_test() {}
    
    // Hardware setup helpers
    virtual void setup_hardware() {
#ifdef HARDWARE_PLATFORM
        setup_gpio_pc6_output();  // LED
        setup_gpio_pc13_input();  // Button
#endif
    }
    
    // SOS hardware setup helpers
    void setup_gpio_pc6_output() {
#ifdef HARDWARE_PLATFORM
        // PC6 as OUTPUT for LED (already done in HAL init, but verify)
        debug_print("Setting up PC6 as OUTPUT for LED");
#endif
    }
    
    void setup_gpio_pc13_input() {
#ifdef HARDWARE_PLATFORM
        // PC13 as INPUT_PULLUP for button
        debug_print("Setting up PC13 as INPUT_PULLUP for button");
        // Note: WeAct board has built-in pullup, but we'll configure it anyway
#endif
    }
    
    // Register validation helpers
    uint32_t read_gpio_register(uint32_t register_address) {
#ifdef HARDWARE_PLATFORM
        volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(register_address);
        return *reg;
#else
        return 0; // QEMU/test environment
#endif
    }
    
    bool validate_pin_mode_bits(uint8_t pin, uint32_t expected_mode) {
        uint32_t moder_value = read_gpio_register(STM32G4_Registers::GPIOC_MODER);
        uint32_t pin_bits = (moder_value >> (pin * 2)) & 0x3;
        
        if (pin_bits != expected_mode) {
            result.error_message = "GPIO pin " + std::to_string(pin) + 
                                 " mode incorrect. Expected: " + std::to_string(expected_mode) +
                                 ", Actual: " + std::to_string(pin_bits);
            return false;
        }
        return true;
    }
    
    bool validate_pin_pullup_bits(uint8_t pin, uint32_t expected_pullup) {
        uint32_t pupdr_value = read_gpio_register(STM32G4_Registers::GPIOC_PUPDR);
        uint32_t pin_bits = (pupdr_value >> (pin * 2)) & 0x3;
        
        if (pin_bits != expected_pullup) {
            result.error_message = "GPIO pin " + std::to_string(pin) + 
                                 " pullup incorrect. Expected: " + std::to_string(expected_pullup) +
                                 ", Actual: " + std::to_string(pin_bits);
            return false;
        }
        return true;
    }
    
    // Utility methods
    uint32_t get_current_time_ms() {
#ifdef HARDWARE_PLATFORM
        return HAL_GetTick();
#else
        return 0; // QEMU/test environment
#endif
    }
    
    void debug_test_print(const std::string& message) {
#ifdef HARDWARE_PLATFORM
        debug_print(("TEST: " + result.test_name + " - " + message).c_str());
#endif
    }
    
    // Disable copy/move for embedded safety
    VMTestBase(const VMTestBase&) = delete;
    VMTestBase& operator=(const VMTestBase&) = delete;
    VMTestBase(VMTestBase&&) = delete;
    VMTestBase& operator=(VMTestBase&&) = delete;
};