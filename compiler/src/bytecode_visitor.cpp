#include "bytecode_visitor.h"
#include "ArduinoCParser.h"
#include <iostream>
#include <unordered_map>

BytecodeVisitor::BytecodeVisitor() : hasErrors(false) {
}

void BytecodeVisitor::emitInstruction(VMOpcode opcode, uint8_t immediate) {
    bytecode.emplace_back(opcode, immediate);
}

void BytecodeVisitor::emitPushConstant(int32_t value) {
    // For 32-bit values, we need to push them in parts
    // For now, assume values fit in 8 bits (Arduino typical)
    if (value >= 0 && value <= 255) {
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint8_t>(value));
    } else {
        // Handle larger values by pushing bytes
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint8_t>(value & 0xFF));
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint8_t>((value >> 8) & 0xFF));
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint8_t>((value >> 16) & 0xFF));
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint8_t>((value >> 24) & 0xFF));
    }
}

void BytecodeVisitor::emitLoadVariable(const std::string& name) {
    Symbol* symbol = symbolTable.lookupSymbol(name);
    if (!symbol) {
        reportError("Undefined variable: " + name);
        return;
    }
    
    if (symbol->isGlobal) {
        emitInstruction(VMOpcode::OP_LOAD_GLOBAL, symbol->globalIndex);
    } else {
        emitInstruction(VMOpcode::OP_LOAD_LOCAL, symbol->stackOffset);
    }
}

void BytecodeVisitor::emitStoreVariable(const std::string& name) {
    Symbol* symbol = symbolTable.lookupSymbol(name);
    if (!symbol) {
        reportError("Undefined variable: " + name);
        return;
    }
    
    if (symbol->isGlobal) {
        emitInstruction(VMOpcode::OP_STORE_GLOBAL, symbol->globalIndex);
    } else {
        emitInstruction(VMOpcode::OP_STORE_LOCAL, symbol->stackOffset);
    }
}

VMOpcode BytecodeVisitor::getArduinoOpcode(const std::string& functionName) {
    static const std::unordered_map<std::string, VMOpcode> opcodeMap = {
        {"pinMode", VMOpcode::OP_PIN_MODE},
        {"digitalWrite", VMOpcode::OP_DIGITAL_WRITE},
        {"digitalRead", VMOpcode::OP_DIGITAL_READ},
        {"analogWrite", VMOpcode::OP_ANALOG_WRITE},
        {"analogRead", VMOpcode::OP_ANALOG_READ},
        {"delay", VMOpcode::OP_DELAY},
        {"millis", VMOpcode::OP_MILLIS},
        {"micros", VMOpcode::OP_MICROS},
        {"printf", VMOpcode::OP_PRINTF}
    };
    
    auto it = opcodeMap.find(functionName);
    if (it != opcodeMap.end()) {
        return it->second;
    }
    
    reportError("Unknown Arduino function: " + functionName);
    return VMOpcode::OP_HALT; // Default fallback
}

int BytecodeVisitor::addStringLiteral(const std::string& str) {
    stringLiterals.push_back(str);
    return stringLiterals.size() - 1;
}

void BytecodeVisitor::reportError(const std::string& message) {
    hasErrors = true;
    errorMessages.push_back(message);
    std::cerr << "Error: " << message << std::endl;
}

antlrcpp::Any BytecodeVisitor::visitProgram(ArduinoCParser::ProgramContext *ctx) {
    std::cout << "Compiling Arduino C program..." << std::endl;
    
    // Visit all declarations and functions
    for (auto child : ctx->children) {
        visit(child);
    }
    
    // Add halt instruction at the end
    emitInstruction(VMOpcode::OP_HALT);
    
    std::cout << "Compilation complete. Generated " << bytecode.size() << " instructions." << std::endl;
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitDeclaration(ArduinoCParser::DeclarationContext *ctx) {
    std::string typeName = ctx->type()->getText();
    std::string varName = ctx->IDENTIFIER()->getText();
    
    DataType dataType = (typeName == "int") ? DataType::INT : DataType::VOID;
    
    if (!symbolTable.declareSymbol(varName, SymbolType::VARIABLE, dataType)) {
        reportError("Variable already declared: " + varName);
    } else {
        std::cout << "Declared variable: " << varName << " (" << typeName << ")" << std::endl;
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitFunctionDefinition(ArduinoCParser::FunctionDefinitionContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    std::string returnType = ctx->type()->getText();
    
    std::cout << "Compiling function: " << funcName << std::endl;
    
    // Enter function scope
    symbolTable.enterScope();
    symbolTable.resetStackOffset();
    
    // Process function body
    visit(ctx->compoundStatement());
    
    // Exit function scope
    symbolTable.exitScope();
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitCompoundStatement(ArduinoCParser::CompoundStatementContext *ctx) {
    // Process all statements in the compound block
    for (auto stmt : ctx->statement()) {
        visit(stmt);
    }
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitExpressionStatement(ArduinoCParser::ExpressionStatementContext *ctx) {
    if (ctx->expression()) {
        visit(ctx->expression());
    }
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitAssignment(ArduinoCParser::AssignmentContext *ctx) {
    std::string varName = ctx->IDENTIFIER()->getText();
    
    // Generate code for the right-hand side expression
    visit(ctx->expression());
    
    // Store the result in the variable
    emitStoreVariable(varName);
    
    std::cout << "Generated assignment: " << varName << " = <expression>" << std::endl;
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitFunctionCall(ArduinoCParser::FunctionCallContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    
    // Process arguments first (push them onto stack)
    if (ctx->argumentList()) {
        for (auto arg : ctx->argumentList()->expression()) {
            visit(arg);
        }
    }
    
    // Generate appropriate instruction
    if (funcName == "analogRead" || funcName == "digitalRead" || 
        funcName == "millis" || funcName == "micros") {
        // Functions that return values
        VMOpcode opcode = getArduinoOpcode(funcName);
        emitInstruction(opcode);
    } else {
        // Functions that don't return values or are more complex
        if (funcName == "printf") {
            // Handle printf specially - need string literal handling
            VMOpcode opcode = getArduinoOpcode(funcName);
            emitInstruction(opcode);
        } else {
            VMOpcode opcode = getArduinoOpcode(funcName);
            emitInstruction(opcode);
        }
    }
    
    std::cout << "Generated function call: " << funcName << std::endl;
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitExpression(ArduinoCParser::ExpressionContext *ctx) {
    if (ctx->assignment()) {
        return visit(ctx->assignment());
    } else if (ctx->functionCall()) {
        return visit(ctx->functionCall());
    } else if (ctx->IDENTIFIER()) {
        // Load variable value onto stack
        std::string varName = ctx->IDENTIFIER()->getText();
        emitLoadVariable(varName);
    } else if (ctx->INTEGER()) {
        // Push integer constant onto stack
        int value = std::stoi(ctx->INTEGER()->getText());
        emitPushConstant(value);
    } else if (ctx->STRING()) {
        // Handle string literal
        std::string str = ctx->STRING()->getText();
        // Remove quotes
        str = str.substr(1, str.length() - 2);
        int stringIndex = addStringLiteral(str);
        emitPushConstant(stringIndex);
    }
    return nullptr;
}

void BytecodeVisitor::printBytecode() const {
    std::cout << "\nGenerated Bytecode:\n";
    for (size_t i = 0; i < bytecode.size(); ++i) {
        const auto& instr = bytecode[i];
        std::cout << i << ": 0x" << std::hex 
                  << static_cast<int>(instr.opcode) << " "
                  << static_cast<int>(instr.immediate) << std::dec
                  << " (encoded: 0x" << std::hex << instr.encode() << std::dec << ")"
                  << std::endl;
    }
    
    if (!stringLiterals.empty()) {
        std::cout << "\nString Literals:\n";
        for (size_t i = 0; i < stringLiterals.size(); ++i) {
            std::cout << i << ": \"" << stringLiterals[i] << "\"" << std::endl;
        }
    }
}

void BytecodeVisitor::printSymbolTable() const {
    symbolTable.printSymbols();
}