#ifndef BYTECODE_VISITOR_H
#define BYTECODE_VISITOR_H

#include "ArduinoCBaseVisitor.h"
#include "symbol_table.h"
#include "../shared/vm_opcodes.h"
#include <vector>
#include <string>
#include <cstdint>
#include <map>

// Flag definitions for instruction variants
enum class InstructionFlag : uint8_t {
    NONE = 0x00,
    SIGNED = 0x01,
    WIDE = 0x02,
    VOLATILE = 0x04,
    CONDITION = 0x08,
    ATOMIC = 0x10,
    DEBUG = 0x20,
    RESERVED1 = 0x40,
    RESERVED2 = 0x80
};

// ARM Cortex-M4 optimized 32-bit instruction format
struct Instruction {
    uint8_t opcode;     // 256 base operations
    uint8_t flags;      // 8 modifier bits for instruction variants
    uint16_t immediate; // 0-65535 range
    
    Instruction(VMOpcode op, uint16_t imm = 0, InstructionFlag flag = InstructionFlag::NONE) 
        : opcode(static_cast<uint8_t>(op)), flags(static_cast<uint8_t>(flag)), immediate(imm) {}
    
    uint32_t encode() const {
        return (static_cast<uint32_t>(opcode) << 24) | 
               (static_cast<uint32_t>(flags) << 16) | 
               static_cast<uint32_t>(immediate);
    }
} __attribute__((packed));

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
    
    void emitInstruction(VMOpcode opcode, uint16_t immediate = 0, InstructionFlag flags = InstructionFlag::NONE);
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
    antlrcpp::Any visitFunctionDeclaration(ArduinoCParser::FunctionDeclarationContext *ctx) override;
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
    antlrcpp::Any visitMultiplicativeExpression(ArduinoCParser::MultiplicativeExpressionContext *ctx) override;
    antlrcpp::Any visitPrimaryExpression(ArduinoCParser::PrimaryExpressionContext *ctx) override;
    
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