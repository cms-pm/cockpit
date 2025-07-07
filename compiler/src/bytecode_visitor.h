#ifndef BYTECODE_VISITOR_H
#define BYTECODE_VISITOR_H

#include "ArduinoCBaseVisitor.h"
#include "symbol_table.h"
#include <vector>
#include <string>
#include <cstdint>
#include <map>

// VM Opcodes (from existing VM implementation)
enum class VMOpcode : uint8_t {
    // Core VM operations (0x01-0x08)
    OP_PUSH = 0x01,
    OP_POP = 0x02,
    OP_ADD = 0x03,
    OP_SUB = 0x04,
    OP_MUL = 0x05,
    OP_DIV = 0x06,
    OP_MOD = 0x07,
    OP_CALL = 0x08,
    OP_RET = 0x09,
    OP_HALT = 0x0A,
    
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
    
    // Control flow operations (0x30-0x3F)
    OP_JMP = 0x30,
    OP_JMP_TRUE = 0x31,
    OP_JMP_FALSE = 0x32,
    
    // Logical operations (0x40-0x4F)
    OP_AND = 0x40,
    OP_OR = 0x41,
    OP_NOT = 0x42,
    
    // Bitwise operations (0x60-0x6F)
    OP_BITWISE_AND = 0x60,
    OP_BITWISE_OR = 0x61,
    OP_BITWISE_XOR = 0x62,
    OP_BITWISE_NOT = 0x63,
    OP_SHIFT_LEFT = 0x64,
    OP_SHIFT_RIGHT = 0x65,
    
    // Memory operations (custom for compiler)
    OP_LOAD_GLOBAL = 0x50,
    OP_STORE_GLOBAL = 0x51,
    OP_LOAD_LOCAL = 0x52,
    OP_STORE_LOCAL = 0x53
};

struct Instruction {
    VMOpcode opcode;
    uint8_t immediate;
    
    Instruction(VMOpcode op, uint8_t imm = 0) : opcode(op), immediate(imm) {}
    
    uint16_t encode() const {
        return (static_cast<uint16_t>(opcode) << 8) | immediate;
    }
};

// Jump placeholder for backpatching
struct JumpPlaceholder {
    size_t instruction_index;  // Where to patch the offset
    std::string target_label;  // Label to resolve
    
    JumpPlaceholder(size_t index, const std::string& label) 
        : instruction_index(index), target_label(label) {}
};

class BytecodeVisitor : public ArduinoCBaseVisitor {
private:
    SymbolTable symbolTable;
    std::vector<Instruction> bytecode;
    std::vector<std::string> stringLiterals;
    bool hasErrors;
    std::vector<std::string> errorMessages;
    
    // Jump resolution system
    std::vector<JumpPlaceholder> jumpPlaceholders;
    std::map<std::string, size_t> labels;
    int labelCounter;
    
    // Function resolution system
    std::map<std::string, size_t> functionAddresses;
    std::vector<JumpPlaceholder> functionCallPlaceholders;
    
    void emitInstruction(VMOpcode opcode, uint8_t immediate = 0);
    void emitPushConstant(int32_t value);
    void emitLoadVariable(const std::string& name);
    void emitStoreVariable(const std::string& name);
    VMOpcode getArduinoOpcode(const std::string& functionName);
    int addStringLiteral(const std::string& str);
    
    // Jump and label management
    std::string generateLabel(const std::string& prefix);
    void emitJump(VMOpcode jumpOpcode, const std::string& targetLabel);
    void placeLabel(const std::string& label);
    void resolveJumps();
    VMOpcode getComparisonOpcode(const std::string& operator_);
    
    // Function address management
    void registerFunction(const std::string& functionName, size_t address);
    void emitFunctionCall(const std::string& functionName);
    void resolveFunctionCalls();
    
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
    
    // Control flow visitor methods
    antlrcpp::Any visitIfStatement(ArduinoCParser::IfStatementContext *ctx) override;
    antlrcpp::Any visitWhileStatement(ArduinoCParser::WhileStatementContext *ctx) override;
    antlrcpp::Any visitConditionalExpression(ArduinoCParser::ConditionalExpressionContext *ctx) override;
    
    // Function and expression visitor methods
    antlrcpp::Any visitReturnStatement(ArduinoCParser::ReturnStatementContext *ctx) override;
    antlrcpp::Any visitArithmeticExpression(ArduinoCParser::ArithmeticExpressionContext *ctx) override;
    
    // Logical expression visitor methods
    antlrcpp::Any visitLogicalOrExpression(ArduinoCParser::LogicalOrExpressionContext *ctx) override;
    antlrcpp::Any visitLogicalAndExpression(ArduinoCParser::LogicalAndExpressionContext *ctx) override;
    antlrcpp::Any visitLogicalNotExpression(ArduinoCParser::LogicalNotExpressionContext *ctx) override;
    
    // Bitwise expression visitor methods
    antlrcpp::Any visitBitwiseOrExpression(ArduinoCParser::BitwiseOrExpressionContext *ctx) override;
    antlrcpp::Any visitBitwiseXorExpression(ArduinoCParser::BitwiseXorExpressionContext *ctx) override;
    antlrcpp::Any visitBitwiseAndExpression(ArduinoCParser::BitwiseAndExpressionContext *ctx) override;
    antlrcpp::Any visitShiftExpression(ArduinoCParser::ShiftExpressionContext *ctx) override;
    
    // Result access
    const std::vector<Instruction>& getBytecode() const { return bytecode; }
    const std::vector<std::string>& getStringLiterals() const { return stringLiterals; }
    bool getHasErrors() const { return hasErrors; }
    const std::vector<std::string>& getErrorMessages() const { return errorMessages; }
    
    void printBytecode() const;
    void printSymbolTable() const;
};

#endif // BYTECODE_VISITOR_H