#include <iostream>
#include <fstream>
#include <string>
#include "antlr4-runtime.h"
#include "ArduinoCLexer.h"
#include "ArduinoCParser.h"
#include "bytecode_visitor.h"

using namespace antlr4;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.c>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return 1;
    }
    
    // Read file content
    std::string input((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "Compiling: " << filename << std::endl;
    std::cout << "Source code:\n" << input << std::endl;
    
    try {
        // Create ANTLR input stream
        ANTLRInputStream inputStream(input);
        
        // Create lexer
        ArduinoCLexer lexer(&inputStream);
        CommonTokenStream tokens(&lexer);
        
        // Create parser
        ArduinoCParser parser(&tokens);
        
        // Parse starting from the program rule
        tree::ParseTree* tree = parser.program();
        
        if (parser.getNumberOfSyntaxErrors() > 0) {
            std::cerr << "Syntax errors found. Compilation failed." << std::endl;
            return 1;
        }
        
        std::cout << "Parse tree: " << tree->toStringTree(&parser) << std::endl;
        
        // Create visitor and generate bytecode
        BytecodeVisitor visitor;
        visitor.visit(tree);
        
        if (visitor.getHasErrors()) {
            std::cerr << "Compilation errors found:" << std::endl;
            for (const auto& error : visitor.getErrorMessages()) {
                std::cerr << "  " << error << std::endl;
            }
            return 1;
        }
        
        // Print results
        visitor.printSymbolTable();
        visitor.printBytecode();
        
        // Save bytecode to file
        std::string outputFile = filename.substr(0, filename.find_last_of('.')) + ".bin";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (outFile.is_open()) {
            const auto& bytecode = visitor.getBytecode();
            for (const auto& instr : bytecode) {
                uint32_t encoded = instr.encode();
                outFile.write(reinterpret_cast<const char*>(&encoded), sizeof(encoded));
            }
            outFile.close();
            std::cout << "Bytecode saved to: " << outputFile << std::endl;
        }
        
        std::cout << "Compilation successful!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Compilation error: " << e.what() << std::endl;
        return 1;
    }
}