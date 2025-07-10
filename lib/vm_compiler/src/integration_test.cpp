#include "vm_integration.h"
#include "bytecode_visitor.h"
#include "ArduinoCLexer.h"
#include "ArduinoCParser.h"
#include <iostream>
#include <sstream>

using namespace antlr4;

int main() {
    std::cout << "Compiler-VM Integration Test\n";
    std::cout << "============================\n";
    
    // Test program source
    std::string source = R"(
        int main() {
            int value = 5000;
            int result = value + 2000;
            return result;
        }
    )";
    
    std::cout << "Source code:\n" << source << "\n";
    
    try {
        // Parse and compile the source
        ANTLRInputStream inputStream(source);
        ArduinoCLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        ArduinoCParser parser(&tokens);
        
        tree::ParseTree* tree = parser.program();
        
        if (parser.getNumberOfSyntaxErrors() > 0) {
            std::cerr << "Syntax errors found" << std::endl;
            return 1;
        }
        
        // Generate bytecode
        BytecodeVisitor visitor;
        visitor.visit(tree);
        
        if (visitor.getHasErrors()) {
            std::cerr << "Compilation errors found:" << std::endl;
            for (const auto& error : visitor.getErrorMessages()) {
                std::cerr << "  " << error << std::endl;
            }
            return 1;
        }
        
        // Display compiled bytecode
        std::cout << "\nCompiler Output:\n";
        visitor.printBytecode();
        
        // Create VM integration and load program
        VMIntegration vm_integration;
        
        std::cout << "\nLoading program into ComponentVM...\n";
        if (!vm_integration.load_program_from_bytecode(visitor.getBytecode())) {
            std::cerr << "Failed to load program into VM" << std::endl;
            return 1;
        }
        
        std::cout << "Program loaded successfully!\n";
        
        // Execute the program
        std::cout << "\nExecuting program...\n";
        bool execution_success = vm_integration.execute_program();
        
        if (execution_success) {
            std::cout << "Program executed successfully!\n";
        } else {
            std::cerr << "Program execution failed: " 
                      << vm_integration.get_error_string() << std::endl;
            return 1;
        }
        
        // Display performance metrics
        auto metrics = vm_integration.get_performance_metrics();
        std::cout << "\nPerformance Metrics:\n";
        std::cout << "  Execution time: " << metrics.execution_time_ms << " ms\n";
        std::cout << "  Instructions executed: " << metrics.instructions_executed << "\n";
        std::cout << "  VM halted: " << (vm_integration.is_halted() ? "Yes" : "No") << "\n";
        
        // Test component access
        const auto& vm = vm_integration.get_vm();
        std::cout << "\nVM Component State:\n";
        std::cout << "  Memory globals: " << vm.get_memory_manager().get_global_count() << "\n";
        std::cout << "  IO initialized: " << vm.get_io_controller().is_hardware_initialized() << "\n";
        std::cout << "  Execution engine PC: " << vm.get_execution_engine().get_pc() << "\n";
        
        std::cout << "\nIntegration test completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Integration test error: " << e.what() << std::endl;
        return 1;
    }
}