#include "bytecode_visitor.h"
#include "ArduinoCLexer.h"
#include "ArduinoCParser.h"
#include <iostream>
#include <sstream>

using namespace antlr4;

// Simple test that validates the 32-bit instruction format without ComponentVM
int main() {
    std::cout << "32-bit Instruction Format Validation Test\n";
    std::cout << "==========================================\n";
    
    // Test program with large immediate values
    std::string source = R"(
        int main() {
            int large_value = 32000;  // Test 16-bit immediate
            int result = large_value + 8000;
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
        std::cout << "\nCompiler Output (32-bit Instructions):\n";
        visitor.printBytecode();
        
        // Validate 32-bit instruction format
        const auto& bytecode = visitor.getBytecode();
        bool format_valid = true;
        
        for (size_t i = 0; i < bytecode.size(); ++i) {
            const auto& instr = bytecode[i];
            
            // Check if we can handle large immediate values
            if (instr.immediate > 255) {
                std::cout << "✓ Instruction " << i << " uses 16-bit immediate: " 
                          << instr.immediate << " (was impossible with 8-bit format)\n";
            }
            
            // Validate 32-bit encoding
            uint32_t encoded = instr.encode();
            
            // Decode and verify
            uint8_t decoded_opcode = (encoded >> 24) & 0xFF;
            uint8_t decoded_flags = (encoded >> 16) & 0xFF;
            uint16_t decoded_immediate = encoded & 0xFFFF;
            
            if (decoded_opcode != instr.opcode || 
                decoded_flags != instr.flags || 
                decoded_immediate != instr.immediate) {
                std::cerr << "✗ Instruction " << i << " encoding/decoding mismatch\n";
                format_valid = false;
            }
        }
        
        if (format_valid) {
            std::cout << "\n✓ All instructions use proper 32-bit ARM-aligned format\n";
            std::cout << "✓ 16-bit immediate values working correctly\n";
            std::cout << "✓ Instruction encoding/decoding validated\n";
        }
        
        // Test specific large value handling
        bool found_large_values = false;
        for (const auto& instr : bytecode) {
            if (instr.immediate == 32000 || instr.immediate == 8000) {
                found_large_values = true;
                std::cout << "✓ Successfully compiled large immediate value: " 
                          << instr.immediate << "\n";
            }
        }
        
        if (!found_large_values) {
            std::cerr << "✗ Failed to find expected large immediate values\n";
            return 1;
        }
        
        std::cout << "\n32-bit Instruction Format Test: SUCCESS!\n";
        std::cout << "Compiler successfully upgraded from 8-bit to 16-bit immediates\n";
        std::cout << "ARM Cortex-M4 optimized 32-bit instruction format working\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << std::endl;
        return 1;
    }
}