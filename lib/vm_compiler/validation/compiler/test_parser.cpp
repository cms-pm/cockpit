#include <iostream>
#include <string>
#include <cassert>
#include "antlr4-runtime.h"
#include "ArduinoCLexer.h"
#include "ArduinoCParser.h"
#include "../src/bytecode_visitor.h"

using namespace antlr4;

void testBasicParsing() {
    std::cout << "Testing basic parsing..." << std::endl;
    
    std::string input = R"(
        int sensorValue;
        
        void setup() {
            pinMode(13, 1);
            sensorValue = analogRead(0);
            digitalWrite(13, 1);
            printf("Sensor: %d\n", sensorValue);
        }
    )";
    
    try {
        ANTLRInputStream inputStream(input);
        ArduinoCLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        ArduinoCParser parser(&tokens);
        
        tree::ParseTree* tree = parser.program();
        
        assert(parser.getNumberOfSyntaxErrors() == 0);
        std::cout << "✓ Parsing successful" << std::endl;
        
        // Test visitor
        BytecodeVisitor visitor;
        visitor.visit(tree);
        
        assert(!visitor.getHasErrors());
        std::cout << "✓ Bytecode generation successful" << std::endl;
        
        const auto& bytecode = visitor.getBytecode();
        assert(!bytecode.empty());
        std::cout << "✓ Generated " << bytecode.size() << " instructions" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        assert(false);
    }
}

void testSymbolTable() {
    std::cout << "Testing symbol table..." << std::endl;
    
    std::string input = R"(
        int globalVar;
        
        void testFunc() {
            int localVar;
            localVar = 42;
            globalVar = localVar;
        }
    )";
    
    try {
        ANTLRInputStream inputStream(input);
        ArduinoCLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        ArduinoCParser parser(&tokens);
        
        tree::ParseTree* tree = parser.program();
        assert(parser.getNumberOfSyntaxErrors() == 0);
        
        BytecodeVisitor visitor;
        visitor.visit(tree);
        
        assert(!visitor.getHasErrors());
        std::cout << "✓ Symbol table management successful" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        assert(false);
    }
}

void testArduinoFunctions() {
    std::cout << "Testing Arduino function calls..." << std::endl;
    
    std::string input = R"(
        void test() {
            pinMode(13, 1);
            digitalWrite(13, 1);
            analogRead(0);
            delay(1000);
            millis();
        }
    )";
    
    try {
        ANTLRInputStream inputStream(input);
        ArduinoCLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        ArduinoCParser parser(&tokens);
        
        tree::ParseTree* tree = parser.program();
        assert(parser.getNumberOfSyntaxErrors() == 0);
        
        BytecodeVisitor visitor;
        visitor.visit(tree);
        
        assert(!visitor.getHasErrors());
        std::cout << "✓ Arduino function compilation successful" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        assert(false);
    }
}

int main() {
    std::cout << "Running Arduino C Compiler Tests\n" << std::endl;
    
    testBasicParsing();
    testSymbolTable();
    testArduinoFunctions();
    
    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}