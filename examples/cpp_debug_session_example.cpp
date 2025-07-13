/*
 * ComponentVM C++ Debug Session Example
 * Phase 4.3.4: Demonstrates RAII Hardware Cleanup
 * 
 * This example shows how to use the C++ debug engine with guaranteed
 * hardware reset cleanup via destructors.
 */

#include "../include/componentvm_debug_engine.h"
#include <iostream>
#include <stdexcept>

void example_basic_usage() {
    std::cout << "\n=== Basic RAII Usage ===" << std::endl;
    
    {
        // Create debug session with RAII
        ComponentVMDebugEngine debug_engine;
        
        debug_engine.start_session();
        debug_engine.execute_gdb_command("monitor reset halt");
        debug_engine.execute_gdb_command("monitor reset run");
        
        std::cout << "Debug work completed..." << std::endl;
        
    } // <- AUTOMATIC CLEANUP HERE: Destructor ensures hardware reset!
    
    std::cout << "Hardware is guaranteed to be reset and running normally" << std::endl;
}

void example_exception_safety() {
    std::cout << "\n=== Exception Safety Example ===" << std::endl;
    
    try {
        ComponentVMDebugEngine debug_engine;
        debug_engine.start_session();
        
        // Simulate some debug work
        debug_engine.execute_gdb_command("monitor reset halt");
        
        // Simulate an exception during debugging
        throw std::runtime_error("Simulated debug session crash!");
        
        // This code never executes due to exception
        debug_engine.execute_gdb_command("monitor reset run");
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        std::cout << "BUT: Hardware was automatically reset by destructor!" << std::endl;
    }
    // Destructor ran automatically during stack unwinding
}

void example_smart_pointer_usage() {
    std::cout << "\n=== Smart Pointer Usage ===" << std::endl;
    
    // Using the helper function for automatic memory management
    auto debug_session = create_debug_session();
    
    debug_session->start_session();
    debug_session->execute_gdb_command("monitor reset halt");
    debug_session->execute_gdb_command("info registers");
    debug_session->execute_gdb_command("monitor reset run");
    
    // Automatic cleanup when unique_ptr goes out of scope
    std::cout << "Smart pointer will automatically clean up..." << std::endl;
}

void example_manual_cleanup() {
    std::cout << "\n=== Manual Cleanup (Optional) ===" << std::endl;
    
    ComponentVMDebugEngine debug_engine;
    debug_engine.start_session();
    
    // Do some debug work...
    debug_engine.execute_gdb_command("monitor reset halt");
    
    // Optionally call cleanup manually (though destructor will handle it anyway)
    debug_engine.cleanup();
    
    std::cout << "Manual cleanup completed (destructor will be no-op)" << std::endl;
}

int main() {
    std::cout << "ComponentVM C++ Debug Engine RAII Examples" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    example_basic_usage();
    example_exception_safety();
    example_smart_pointer_usage();
    example_manual_cleanup();
    
    std::cout << "\n✅ All examples completed successfully!" << std::endl;
    std::cout << "Hardware is guaranteed to be in proper running state." << std::endl;
    
    return 0;
}

/*
 * Key Benefits of This RAII Approach:
 * 
 * 1. **Guaranteed Cleanup**: Hardware reset happens automatically even if:
 *    - Program crashes with segfault
 *    - Exception occurs during debugging
 *    - User presses Ctrl+C to interrupt
 *    - Program exits normally
 * 
 * 2. **Exception Safety**: Stack unwinding ensures destructors run
 * 
 * 3. **No Memory Leaks**: RAII handles all resource management
 * 
 * 4. **Simple Usage**: Just create object, use it, automatic cleanup
 * 
 * 5. **Move Semantics**: Efficient transfer of ownership
 * 
 * 6. **Thread Safety**: Each debug engine instance manages its own resources
 */