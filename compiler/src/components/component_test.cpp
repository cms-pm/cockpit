#include "component_vm.h"
#include <iostream>
#include <array>

// Simple test program: push 42, push 24, add, halt
constexpr std::array<VM::Instruction, 4> test_program = {{
    {0x01, 0x00, 42},   // PUSH 42
    {0x01, 0x00, 24},   // PUSH 24
    {0x03, 0x00, 0},    // ADD
    {0x00, 0x00, 0}     // HALT
}};

int main() {
    std::cout << "Component VM Test Starting...\n";
    
    // Create ComponentVM instance
    ComponentVM vm;
    
    std::cout << "VM created successfully\n";
    
    // Test component access
    std::cout << "Memory manager globals: " << vm.get_memory_manager().get_global_count() << "\n";
    std::cout << "IO controller initialized: " << vm.get_io_controller().is_hardware_initialized() << "\n";
    std::cout << "Execution engine halted: " << vm.get_execution_engine().is_halted() << "\n";
    
    // Test program execution
    std::cout << "Loading test program...\n";
    bool loaded = vm.load_program(test_program.data(), test_program.size());
    std::cout << "Program loaded: " << (loaded ? "success" : "failed") << "\n";
    
    if (loaded) {
        std::cout << "Executing program...\n";
        bool executed = vm.execute_program(test_program.data(), test_program.size());
        std::cout << "Program executed: " << (executed ? "success" : "failed") << "\n";
        
        if (!executed) {
            std::cout << "Error: " << vm.get_error_string(vm.get_last_error()) << "\n";
        }
        
        // Check final state
        std::cout << "VM halted: " << vm.is_halted() << "\n";
        std::cout << "Instructions executed: " << vm.get_performance_metrics().instructions_executed << "\n";
    }
    
    std::cout << "Component VM Test Completed\n";
    return 0;
}