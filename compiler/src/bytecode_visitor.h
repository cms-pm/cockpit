#ifndef BYTECODE_VISITOR_H
#define BYTECODE_VISITOR_H

#include "ArduinoCBaseVisitor.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <cstdint>

// VM Opcodes (from existing VM implementation)
enum class VMOpcode : uint8_t {
    // Core VM operations (0x01-0x08)
    OP_PUSH = 0x01,
    OP_POP = 0x02,
    OP_ADD = 0x03,
    OP_SUB = 0x04,
    OP_MUL = 0x05,
    OP_DIV = 0x06,
    OP_CALL = 0x07,
    OP_RET = 0x08,
    OP_HALT = 0x09,
    
    // Arduino functions (0x10-0x1F)
    OP_DIGITAL_WRITE = 0x10,
    OP_DIGITAL_READ = 0x11,
    OP_ANALOG_WRITE = 0x12,
    OP_ANALOG_READ = 0x13,
    OP_DELAY = 0x14,
    OP_BUTTON_PRESSED = 0x15,
    OP_BUTTON_RELEASED = 0x16,
    OP_PIN_MODE = 0x17,
    OP_PRINTF = 0x18,
    OP_MILLIS = 0x19,
    OP_MICROS = 0x1A,
    
    // Comparison operations (0x20-0x2F)
    OP_EQ = 0x20,
    OP_NE = 0x21,
    OP_LT = 0x22,
    OP_GT = 0x23,
    OP_LE = 0x24,
    OP_GE = 0x25,
    
    // Memory operations (custom for compiler)
    OP_LOAD_GLOBAL = 0x40,
    OP_STORE_GLOBAL = 0x41,
    OP_LOAD_LOCAL = 0x42,
    OP_STORE_LOCAL = 0x43
};

struct Instruction {
    VMOpcode opcode;
    uint8_t immediate;
    
    Instruction(VMOpcode op, uint8_t imm = 0) : opcode(op), immediate(imm) {}
    
    uint16_t encode() const {
        return (static_cast<uint16_t>(opcode) << 8) | immediate;
    }
};

class BytecodeVisitor : public ArduinoCBaseVisitor {
private:
    SymbolTable symbolTable;
    std::vector<Instruction> bytecode;
    std::vector<std::string> stringLiterals;
    bool hasErrors;
    std::vector<std::string> errorMessages;
    
    void emitInstruction(VMOpcode opcode, uint8_t immediate = 0);
    void emitPushConstant(int32_t value);
    void emitLoadVariable(const std::string& name);
    void emitStoreVariable(const std::string& name);
    VMOpcode getArduinoOpcode(const std::string& functionName);
    int addStringLiteral(const std::string& str);
    
    void reportError(const std::string& message);
    
public:
    BytecodeVisitor();
    
    // ANTLR visitor methods
    antlrcpp::Any visitProgram(ArduinoCParser::ProgramContext *ctx) override;
    antlrcpp::Any visitDeclaration(ArduinoCParser::DeclarationContext *ctx) override;
    antlrcpp::Any visitFunctionDefinition(ArduinoCParser::FunctionDefinitionContext *ctx) override;
    antlrcpp::Any visitCompoundStatement(ArduinoCParser::CompoundStatementContext *ctx) override;
    antlrcpp::Any visitExpressionStatement(ArduinoCParser::ExpressionStatementContext *ctx) override;
    antlrcpp::Any visitAssignment(ArduinoCParser::AssignmentContext *ctx) override;
    antlrcpp::Any visitFunctionCall(ArduinoCParser::FunctionCallContext *ctx) override;
    antlrcpp::Any visitExpression(ArduinoCParser::ExpressionContext *ctx) override;
    
    // Result access
    const std::vector<Instruction>& getBytecode() const { return bytecode; }
    const std::vector<std::string>& getStringLiterals() const { return stringLiterals; }
    bool getHasErrors() const { return hasErrors; }
    const std::vector<std::string>& getErrorMessages() const { return errorMessages; }
    
    void printBytecode() const;
    void printSymbolTable() const;
};

#endif // BYTECODE_VISITOR_H