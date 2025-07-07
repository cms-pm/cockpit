#include "bytecode_visitor.h"
#include "ArduinoCParser.h"
#include <iostream>
#include <unordered_map>

BytecodeVisitor::BytecodeVisitor() : hasErrors(false), labelCounter(0) {
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
    
    // Not an Arduino function - return sentinel value without error
    // Let the caller handle user-defined function resolution
    return VMOpcode::OP_HALT; // Sentinel: not found
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
    
    // Resolve all jump targets and function calls
    resolveJumps();
    resolveFunctionCalls();
    
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
        
        // Handle initialization if present
        if (ctx->expression()) {
            std::cout << "Initializing variable: " << varName << std::endl;
            
            // Evaluate the initialization expression
            visit(ctx->expression());
            
            // Store the result in the variable
            emitStoreVariable(varName);
        }
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitFunctionDefinition(ArduinoCParser::FunctionDefinitionContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    std::string returnType = ctx->type()->getText();
    
    std::cout << "Compiling function: " << funcName << std::endl;
    
    // Register function address before generating body
    size_t function_address = bytecode.size();
    registerFunction(funcName, function_address);
    
    // Declare function in symbol table
    DataType dataType = (returnType == "int") ? DataType::INT : DataType::VOID;
    symbolTable.declareSymbol(funcName, SymbolType::FUNCTION, dataType);
    
    // Enter function scope for parameters and local variables
    symbolTable.enterScope();
    symbolTable.resetStackOffset();
    
    // Process parameters if any
    if (ctx->parameterList()) {
        auto params = ctx->parameterList()->parameter();
        for (auto param : params) {
            std::string paramType = param->type()->getText();
            std::string paramName = param->IDENTIFIER()->getText();
            DataType paramDataType = (paramType == "int") ? DataType::INT : DataType::VOID;
            
            symbolTable.declareSymbol(paramName, SymbolType::PARAMETER, paramDataType);
            std::cout << "Function parameter: " << paramName << " (" << paramType << ")" << std::endl;
        }
    }
    
    // Process function body
    visit(ctx->compoundStatement());
    
    // Generate return instruction if not void function
    if (returnType == "void") {
        emitInstruction(VMOpcode::OP_RET);
    } else {
        // For non-void functions, assume return value is on stack
        emitInstruction(VMOpcode::OP_RET);
    }
    
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
    std::string assignmentText = ctx->getText();
    
    // Check for compound assignment operators
    if (assignmentText.find("+=") != std::string::npos) {
        // Compound addition: var += expr -> var = var + expr
        emitLoadVariable(varName);  // Load current value
        visit(ctx->expression());   // Evaluate right-hand side
        emitInstruction(VMOpcode::OP_ADD);  // Add them
        emitStoreVariable(varName); // Store result
        std::cout << "Generated compound assignment: " << varName << " += <expression>" << std::endl;
        
    } else if (assignmentText.find("-=") != std::string::npos) {
        // Compound subtraction: var -= expr -> var = var - expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_SUB);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " -= <expression>" << std::endl;
        
    } else if (assignmentText.find("*=") != std::string::npos) {
        // Compound multiplication: var *= expr -> var = var * expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_MUL);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " *= <expression>" << std::endl;
        
    } else if (assignmentText.find("/=") != std::string::npos) {
        // Compound division: var /= expr -> var = var / expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_DIV);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " /= <expression>" << std::endl;
        
    } else if (assignmentText.find("%=") != std::string::npos) {
        // Compound modulo: var %= expr -> var = var % expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_MOD);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " %= <expression>" << std::endl;
        
    } else if (assignmentText.find("&=") != std::string::npos) {
        // Compound bitwise AND: var &= expr -> var = var & expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_BITWISE_AND);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " &= <expression>" << std::endl;
        
    } else if (assignmentText.find("|=") != std::string::npos) {
        // Compound bitwise OR: var |= expr -> var = var | expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_BITWISE_OR);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " |= <expression>" << std::endl;
        
    } else if (assignmentText.find("^=") != std::string::npos) {
        // Compound bitwise XOR: var ^= expr -> var = var ^ expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_BITWISE_XOR);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " ^= <expression>" << std::endl;
        
    } else if (assignmentText.find("<<=") != std::string::npos) {
        // Compound left shift: var <<= expr -> var = var << expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_SHIFT_LEFT);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " <<= <expression>" << std::endl;
        
    } else if (assignmentText.find(">>=") != std::string::npos) {
        // Compound right shift: var >>= expr -> var = var >> expr
        emitLoadVariable(varName);
        visit(ctx->expression());
        emitInstruction(VMOpcode::OP_SHIFT_RIGHT);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " >>= <expression>" << std::endl;
        
    } else {
        // Regular assignment: var = expr
        visit(ctx->expression());
        emitStoreVariable(varName);
        std::cout << "Generated assignment: " << varName << " = <expression>" << std::endl;
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitFunctionCall(ArduinoCParser::FunctionCallContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    
    // Process arguments first (push them onto stack in reverse order for correct parameter order)
    if (ctx->argumentList()) {
        auto args = ctx->argumentList()->expression();
        for (auto arg : args) {
            visit(arg);
        }
    }
    
    // Use our unified function call system
    emitFunctionCall(funcName);
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitExpression(ArduinoCParser::ExpressionContext *ctx) {
    if (ctx->assignment()) {
        return visit(ctx->assignment());
    } else if (ctx->logicalOrExpression()) {
        return visit(ctx->logicalOrExpression());
    } else if (ctx->conditionalExpression()) {
        return visit(ctx->conditionalExpression());
    } else if (ctx->arithmeticExpression()) {
        return visit(ctx->arithmeticExpression());
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

// Jump and label management methods
std::string BytecodeVisitor::generateLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(labelCounter++);
}

void BytecodeVisitor::emitJump(VMOpcode jumpOpcode, const std::string& targetLabel) {
    // Emit jump instruction with placeholder offset (0 for now)
    size_t instruction_index = bytecode.size();
    emitInstruction(jumpOpcode, 0);
    
    // Add to jump placeholders for later resolution
    jumpPlaceholders.emplace_back(instruction_index, targetLabel);
}

void BytecodeVisitor::placeLabel(const std::string& label) {
    labels[label] = bytecode.size();
}

void BytecodeVisitor::resolveJumps() {
    for (const auto& placeholder : jumpPlaceholders) {
        const std::string& label = placeholder.target_label;
        
        // Find the label
        auto labelIt = labels.find(label);
        if (labelIt == labels.end()) {
            reportError("Undefined label: " + label);
            continue;
        }
        
        // Calculate jump offset (relative to instruction after the jump)
        size_t jump_instruction_index = placeholder.instruction_index;
        size_t target_index = labelIt->second;
        int32_t offset = static_cast<int32_t>(target_index) - static_cast<int32_t>(jump_instruction_index + 1);
        
        // Check if offset fits in signed 8-bit range
        if (offset < -128 || offset > 127) {
            reportError("Jump offset out of range (-128 to 127): " + std::to_string(offset));
            continue;
        }
        
        // Patch the jump instruction
        bytecode[jump_instruction_index].immediate = static_cast<uint8_t>(static_cast<int8_t>(offset));
    }
    
    // Clear placeholders after resolution
    jumpPlaceholders.clear();
}

VMOpcode BytecodeVisitor::getComparisonOpcode(const std::string& operator_) {
    if (operator_ == "==") return VMOpcode::OP_EQ;
    if (operator_ == "!=") return VMOpcode::OP_NE;
    if (operator_ == "<") return VMOpcode::OP_LT;
    if (operator_ == ">") return VMOpcode::OP_GT;
    if (operator_ == "<=") return VMOpcode::OP_LE;
    if (operator_ == ">=") return VMOpcode::OP_GE;
    
    reportError("Unknown comparison operator: " + operator_);
    return VMOpcode::OP_EQ; // Default fallback
}

// Function address management methods
void BytecodeVisitor::registerFunction(const std::string& functionName, size_t address) {
    functionAddresses[functionName] = address;
    std::cout << "Registered function: " << functionName << " at address " << address << std::endl;
}

void BytecodeVisitor::emitFunctionCall(const std::string& functionName) {
    // Check if it's an Arduino API function first
    VMOpcode arduinoOpcode = getArduinoOpcode(functionName);
    if (arduinoOpcode != VMOpcode::OP_HALT) {
        // It's an Arduino function, emit the dedicated opcode
        emitInstruction(arduinoOpcode);
        return;
    }
    
    // It's a user-defined function, emit OP_CALL with placeholder
    size_t instruction_index = bytecode.size();
    emitInstruction(VMOpcode::OP_CALL, 0);  // Address = 0 (placeholder)
    
    // Add to function call placeholders for later resolution
    functionCallPlaceholders.emplace_back(instruction_index, functionName);
    std::cout << "Generated function call: " << functionName << " (placeholder)" << std::endl;
}

void BytecodeVisitor::resolveFunctionCalls() {
    for (const auto& placeholder : functionCallPlaceholders) {
        const std::string& funcName = placeholder.target_label;
        
        // Find the function address
        auto funcIt = functionAddresses.find(funcName);
        if (funcIt == functionAddresses.end()) {
            reportError("Undefined function: " + funcName);
            continue;
        }
        
        // Calculate function address offset
        size_t function_address = funcIt->second;
        
        // Check if offset fits in 8-bit range
        if (function_address > 255) {
            reportError("Function address out of range (0-255): " + std::to_string(function_address));
            continue;
        }
        
        // Patch the function call instruction
        bytecode[placeholder.instruction_index].immediate = static_cast<uint8_t>(function_address);
        std::cout << "Resolved function call: " << funcName << " to address " << function_address << std::endl;
    }
    
    // Clear placeholders after resolution
    functionCallPlaceholders.clear();
}

// Control flow visitor methods
antlrcpp::Any BytecodeVisitor::visitIfStatement(ArduinoCParser::IfStatementContext *ctx) {
    std::cout << "Compiling if statement" << std::endl;
    
    // Generate labels for control flow
    std::string else_label = generateLabel("else");
    std::string end_label = generateLabel("end_if");
    
    // Visit condition expression
    visit(ctx->expression());
    
    // Jump to else block if condition is false
    if (ctx->statement().size() > 1) {  // Check if else clause exists
        emitJump(VMOpcode::OP_JMP_FALSE, else_label);
        
        // Visit then statement
        visit(ctx->statement(0));
        
        // Jump past else block
        emitJump(VMOpcode::OP_JMP, end_label);
        
        // Place else label
        placeLabel(else_label);
        
        // Visit else statement  
        visit(ctx->statement(1));
        
        // Place end label
        placeLabel(end_label);
    } else {
        // No else clause - just jump to end if condition is false
        emitJump(VMOpcode::OP_JMP_FALSE, end_label);
        
        // Visit then statement
        visit(ctx->statement(0));
        
        // Place end label
        placeLabel(end_label);
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitWhileStatement(ArduinoCParser::WhileStatementContext *ctx) {
    std::cout << "Compiling while statement" << std::endl;
    
    // Generate labels for loop control
    std::string loop_start = generateLabel("while_start");
    std::string loop_end = generateLabel("while_end");
    
    // Place loop start label
    placeLabel(loop_start);
    
    // Visit condition expression
    visit(ctx->expression());
    
    // Jump to end if condition is false
    emitJump(VMOpcode::OP_JMP_FALSE, loop_end);
    
    // Visit loop body
    visit(ctx->statement());
    
    // Jump back to start
    emitJump(VMOpcode::OP_JMP, loop_start);
    
    // Place end label
    placeLabel(loop_end);
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitConditionalExpression(ArduinoCParser::ConditionalExpressionContext *ctx) {
    std::cout << "Compiling conditional expression" << std::endl;
    
    auto shiftExpressions = ctx->shiftExpression();
    
    if (shiftExpressions.size() == 1) {
        // No comparison, just visit the shift expression
        visit(shiftExpressions[0]);
        return nullptr;
    }
    
    // Has comparison operator
    visit(shiftExpressions[0]);  // Left operand
    visit(shiftExpressions[1]);  // Right operand
    
    // Get comparison operator and emit corresponding instruction
    std::string operator_ = ctx->comparisonOperator()->getText();
    VMOpcode compareOp = getComparisonOpcode(operator_);
    emitInstruction(compareOp);
    
    return nullptr;
}

// Function and expression visitor methods
antlrcpp::Any BytecodeVisitor::visitReturnStatement(ArduinoCParser::ReturnStatementContext *ctx) {
    std::cout << "Compiling return statement" << std::endl;
    
    // If there's an expression, evaluate it and leave result on stack
    if (ctx->expression()) {
        visit(ctx->expression());
    }
    
    // Generate return instruction (OP_RET will handle the value on stack)
    emitInstruction(VMOpcode::OP_RET);
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitArithmeticExpression(ArduinoCParser::ArithmeticExpressionContext *ctx) {
    std::cout << "Compiling arithmetic expression (additive)" << std::endl;
    
    auto multiplicativeExpressions = ctx->multiplicativeExpression();
    
    if (multiplicativeExpressions.size() == 1) {
        // Single multiplicative expression, just visit it
        visit(multiplicativeExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with + or - operators
    visit(multiplicativeExpressions[0]);  // First operand
    
    for (size_t i = 1; i < multiplicativeExpressions.size(); i++) {
        visit(multiplicativeExpressions[i]);  // Next operand
        
        // Determine operator based on position in original text
        // This is more robust than getText() scanning
        std::string fullText = ctx->getText();
        if (fullText.find("+") != std::string::npos) {
            emitInstruction(VMOpcode::OP_ADD);
        } else if (fullText.find("-") != std::string::npos) {
            emitInstruction(VMOpcode::OP_SUB);
        }
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitMultiplicativeExpression(ArduinoCParser::MultiplicativeExpressionContext *ctx) {
    std::cout << "Compiling multiplicative expression" << std::endl;
    
    auto primaryExpressions = ctx->primaryExpression();
    
    if (primaryExpressions.size() == 1) {
        // Single primary expression, just visit it
        visit(primaryExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with *, /, or % operators
    visit(primaryExpressions[0]);  // First operand
    
    for (size_t i = 1; i < primaryExpressions.size(); i++) {
        visit(primaryExpressions[i]);  // Next operand
        
        // Determine operator - for MVP, we'll use getText scanning
        // TODO: Improve with proper token access in future iterations
        std::string fullText = ctx->getText();
        if (fullText.find("*") != std::string::npos) {
            emitInstruction(VMOpcode::OP_MUL);
        } else if (fullText.find("/") != std::string::npos) {
            emitInstruction(VMOpcode::OP_DIV);
        } else if (fullText.find("%") != std::string::npos) {
            emitInstruction(VMOpcode::OP_MOD);
        }
    }
    
    return nullptr;
}

// Logical expression visitor methods
antlrcpp::Any BytecodeVisitor::visitLogicalOrExpression(ArduinoCParser::LogicalOrExpressionContext *ctx) {
    std::cout << "Compiling logical OR expression" << std::endl;
    
    // For short-circuit evaluation of OR (a || b):
    // If 'a' is true, result is true (skip evaluation of 'b')
    // If 'a' is false, evaluate 'b' and use its result
    
    auto andExpressions = ctx->logicalAndExpression();
    
    if (andExpressions.size() == 1) {
        // Single operand, just visit it
        visit(andExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with OR operators
    std::string true_label = generateLabel("or_true");
    std::string end_label = generateLabel("or_end");
    
    for (size_t i = 0; i < andExpressions.size(); i++) {
        // Evaluate current operand
        visit(andExpressions[i]);
        
        if (i < andExpressions.size() - 1) {
            // Not the last operand
            // If current operand is true, jump to true result
            emitJump(VMOpcode::OP_JMP_TRUE, true_label);
            // Otherwise, continue to next operand (current false value still on stack)
        }
    }
    
    // If we reach here, last operand result is on stack
    emitJump(VMOpcode::OP_JMP, end_label);
    
    // True label: push true result
    placeLabel(true_label);
    emitInstruction(VMOpcode::OP_POP);  // Remove last operand result
    emitPushConstant(1);  // Push true
    
    placeLabel(end_label);
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitLogicalAndExpression(ArduinoCParser::LogicalAndExpressionContext *ctx) {
    std::cout << "Compiling logical AND expression" << std::endl;
    
    // For short-circuit evaluation of AND (a && b):
    // If 'a' is false, result is false (skip evaluation of 'b')
    // If 'a' is true, evaluate 'b' and use its result
    
    auto notExpressions = ctx->logicalNotExpression();
    
    if (notExpressions.size() == 1) {
        // Single operand, just visit it
        visit(notExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with AND operators
    std::string false_label = generateLabel("and_false");
    std::string end_label = generateLabel("and_end");
    
    for (size_t i = 0; i < notExpressions.size(); i++) {
        // Evaluate current operand
        visit(notExpressions[i]);
        
        if (i < notExpressions.size() - 1) {
            // Not the last operand
            // If current operand is false, jump to false result
            emitJump(VMOpcode::OP_JMP_FALSE, false_label);
            // Otherwise, continue to next operand (current true value still on stack)
        }
    }
    
    // If we reach here, last operand result is on stack
    emitJump(VMOpcode::OP_JMP, end_label);
    
    // False label: push false result
    placeLabel(false_label);
    emitInstruction(VMOpcode::OP_POP);  // Remove last operand result
    emitPushConstant(0);  // Push false
    
    placeLabel(end_label);
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitLogicalNotExpression(ArduinoCParser::LogicalNotExpressionContext *ctx) {
    std::cout << "Compiling logical/bitwise NOT expression" << std::endl;
    
    std::string text = ctx->getText();
    if (text.substr(0, 1) == "!") {
        // This is a logical NOT expression, recursively visit the nested expression
        visit(ctx->logicalNotExpression());
        // Emit logical NOT instruction
        emitInstruction(VMOpcode::OP_NOT);
    } else if (text.substr(0, 1) == "~") {
        // This is a bitwise NOT expression, recursively visit the nested expression
        visit(ctx->logicalNotExpression());
        // Emit bitwise NOT instruction
        emitInstruction(VMOpcode::OP_BITWISE_NOT);
    } else {
        // Not a NOT expression, visit the bitwise OR expression
        visit(ctx->bitwiseOrExpression());
    }
    
    return nullptr;
}

// Bitwise expression visitor methods
antlrcpp::Any BytecodeVisitor::visitBitwiseOrExpression(ArduinoCParser::BitwiseOrExpressionContext *ctx) {
    std::cout << "Compiling bitwise OR expression" << std::endl;
    
    auto xorExpressions = ctx->bitwiseXorExpression();
    
    if (xorExpressions.size() == 1) {
        // Single operand, just visit it
        visit(xorExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with OR operators
    visit(xorExpressions[0]);  // First operand
    
    for (size_t i = 1; i < xorExpressions.size(); i++) {
        visit(xorExpressions[i]);  // Next operand
        emitInstruction(VMOpcode::OP_BITWISE_OR);  // Perform OR operation
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitBitwiseXorExpression(ArduinoCParser::BitwiseXorExpressionContext *ctx) {
    std::cout << "Compiling bitwise XOR expression" << std::endl;
    
    auto andExpressions = ctx->bitwiseAndExpression();
    
    if (andExpressions.size() == 1) {
        // Single operand, just visit it
        visit(andExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with XOR operators
    visit(andExpressions[0]);  // First operand
    
    for (size_t i = 1; i < andExpressions.size(); i++) {
        visit(andExpressions[i]);  // Next operand
        emitInstruction(VMOpcode::OP_BITWISE_XOR);  // Perform XOR operation
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitBitwiseAndExpression(ArduinoCParser::BitwiseAndExpressionContext *ctx) {
    std::cout << "Compiling bitwise AND expression" << std::endl;
    
    auto conditionalExpressions = ctx->conditionalExpression();
    
    if (conditionalExpressions.size() == 1) {
        // Single operand, just visit it
        visit(conditionalExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with AND operators
    visit(conditionalExpressions[0]);  // First operand
    
    for (size_t i = 1; i < conditionalExpressions.size(); i++) {
        visit(conditionalExpressions[i]);  // Next operand
        emitInstruction(VMOpcode::OP_BITWISE_AND);  // Perform AND operation
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitShiftExpression(ArduinoCParser::ShiftExpressionContext *ctx) {
    std::cout << "Compiling shift expression" << std::endl;
    
    auto arithmeticExpressions = ctx->arithmeticExpression();
    
    if (arithmeticExpressions.size() == 1) {
        // Single operand, just visit it
        visit(arithmeticExpressions[0]);
        return nullptr;
    }
    
    // Multiple operands with shift operators
    visit(arithmeticExpressions[0]);  // First operand
    
    for (size_t i = 1; i < arithmeticExpressions.size(); i++) {
        visit(arithmeticExpressions[i]);  // Next operand
        
        // Determine shift direction from the token between operands
        // This is a simplified approach - in a real parser we'd get the operator token
        std::string fullText = ctx->getText();
        if (fullText.find("<<") != std::string::npos) {
            emitInstruction(VMOpcode::OP_SHIFT_LEFT);
        } else if (fullText.find(">>") != std::string::npos) {
            emitInstruction(VMOpcode::OP_SHIFT_RIGHT);
        }
    }
    
    return nullptr;
}