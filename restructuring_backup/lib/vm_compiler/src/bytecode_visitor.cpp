#include "bytecode_visitor.h"
#include "ArduinoCParser.h"
#include <iostream>
#include <unordered_map>

BytecodeVisitor::BytecodeVisitor() : hasErrors(false), labelCounter(0) {
}

void BytecodeVisitor::emitInstruction(VMOpcode opcode, uint16_t immediate, InstructionFlag flags) {
    bytecode.emplace_back(opcode, immediate, flags);
}

void BytecodeVisitor::emitPushConstant(int32_t value) {
    // With 16-bit immediate, we can handle values up to 65535 in single instruction
    if (value >= 0 && value <= 65535) {
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint16_t>(value));
    } else if (value >= -32768 && value < 0) {
        // Negative values within 16-bit signed range
        emitInstruction(VMOpcode::OP_PUSH, static_cast<uint16_t>(value & 0xFFFF), InstructionFlag::SIGNED);
    } else {
        // Large values require multiple instructions (split into 16-bit parts)
        uint16_t low = static_cast<uint16_t>(value & 0xFFFF);
        uint16_t high = static_cast<uint16_t>((value >> 16) & 0xFFFF);
        
        emitInstruction(VMOpcode::OP_PUSH, low);   // Push low 16 bits
        emitInstruction(VMOpcode::OP_PUSH, high);  // Push high 16 bits
        // VM would need to reconstruct 32-bit value from stack
    }
}

void BytecodeVisitor::emitLoadVariable(const std::string& name) {
    Symbol* symbol = symbolTable.lookupSymbol(name);
    if (!symbol) {
        reportError("Undefined variable: " + name);
        return;
    }
    
    if (symbol->isGlobal) {
        emitInstruction(VMOpcode::OP_LOAD_GLOBAL, static_cast<uint16_t>(symbol->globalIndex));
    } else {
        emitInstruction(VMOpcode::OP_LOAD_LOCAL, static_cast<uint16_t>(symbol->stackOffset));
    }
}

void BytecodeVisitor::emitStoreVariable(const std::string& name) {
    Symbol* symbol = symbolTable.lookupSymbol(name);
    if (!symbol) {
        reportError("Undefined variable: " + name);
        return;
    }
    
    if (symbol->isGlobal) {
        emitInstruction(VMOpcode::OP_STORE_GLOBAL, static_cast<uint16_t>(symbol->globalIndex));
    } else {
        emitInstruction(VMOpcode::OP_STORE_LOCAL, static_cast<uint16_t>(symbol->stackOffset));
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
    
    // Check if we have a setup() or main() function and emit entry point
    bool has_setup = false;
    bool has_main = false;
    
    // First pass: scan for entry point functions
    for (auto child : ctx->children) {
        if (auto funcDef = dynamic_cast<ArduinoCParser::FunctionDefinitionContext*>(child)) {
            std::string funcName = funcDef->IDENTIFIER()->getText();
            if (funcName == "setup") has_setup = true;
            if (funcName == "main") has_main = true;
        }
    }
    
    // Generate entry point call
    if (has_main) {
        emitFunctionCall("main");
    } else if (has_setup) {
        emitFunctionCall("setup");
    }
    emitInstruction(VMOpcode::OP_HALT);
    
    // Visit all declarations and functions (they will be placed after entry point)
    for (auto child : ctx->children) {
        visit(child);
    }
    
    // Resolve all jump targets and function calls
    resolveJumps();
    resolveFunctionCalls();
    
    std::cout << "Compilation complete. Generated " << bytecode.size() << " instructions." << std::endl;
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitDeclaration(ArduinoCParser::DeclarationContext *ctx) {
    std::string typeName = ctx->type()->getText();
    std::string varName = ctx->IDENTIFIER()->getText();
    
    DataType dataType = (typeName == "int") ? DataType::INT : DataType::VOID;
    
    // Check if this is an array declaration
    if (ctx->INTEGER()) {
        // Array declaration: int arr[size];
        std::string sizeStr = ctx->INTEGER()->getText();
        size_t arraySize = std::stoul(sizeStr);
        
        if (!symbolTable.declareArray(varName, dataType, arraySize)) {
            reportError("Array already declared: " + varName);
        } else {
            std::cout << "Declared array: " << varName << "[" << arraySize << "] (" << typeName << ")" << std::endl;
            
            // Emit array creation instruction
            Symbol* symbol = symbolTable.lookupSymbol(varName);
            if (symbol) {
                emitInstruction(VMOpcode::OP_CREATE_ARRAY, static_cast<uint16_t>(symbol->arrayId));
                emitPushConstant(static_cast<int32_t>(arraySize));
            }
        }
    } else {
        // Regular variable declaration
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

antlrcpp::Any BytecodeVisitor::visitFunctionDeclaration(ArduinoCParser::FunctionDeclarationContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    std::string returnType = ctx->type()->getText();
    
    std::cout << "Declaring function prototype: " << funcName << std::endl;
    
    // Only declare function in symbol table - no bytecode generation
    // Address will be assigned later when actual definition is encountered
    DataType dataType = (returnType == "int") ? DataType::INT : DataType::VOID;
    symbolTable.declareSymbol(funcName, SymbolType::FUNCTION, dataType);
    
    // Note: We don't call registerFunction() here because no bytecode is generated yet
    // The actual function definition will register the address
    
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
    
    // Get the expression(s) - may be vector due to array access
    auto expressions = ctx->expression();
    
    // Check if this is an array assignment: identifier[index] = value
    if (expressions.size() == 2) {
        // Array assignment: arr[index] = value
        visit(expressions[0]);  // Evaluate index expression
        visit(expressions[1]);  // Evaluate value expression
        
        // Look up array symbol to get array ID
        Symbol* symbol = symbolTable.lookupSymbol(varName);
        if (!symbol) {
            reportError("Undefined array: " + varName);
            return nullptr;
        }
        
        if (symbol->symbolType != SymbolType::ARRAY) {
            reportError("Variable is not an array: " + varName);
            return nullptr;
        }
        
        // Emit array store instruction
        emitInstruction(VMOpcode::OP_STORE_ARRAY, static_cast<uint16_t>(symbol->arrayId));
        std::cout << "Generated array assignment: " << varName << "[index] = value" << std::endl;
        return nullptr;
    }
    
    // Regular variable assignment or compound assignment
    auto expression = expressions[0];  // Single expression for regular assignment
    
    // Check for compound assignment operators
    if (assignmentText.find("+=") != std::string::npos) {
        // Compound addition: var += expr -> var = var + expr
        emitLoadVariable(varName);  // Load current value
        visit(expression);   // Evaluate right-hand side
        emitInstruction(VMOpcode::OP_ADD);  // Add them
        emitStoreVariable(varName); // Store result
        std::cout << "Generated compound assignment: " << varName << " += <expression>" << std::endl;
        
    } else if (assignmentText.find("-=") != std::string::npos) {
        // Compound subtraction: var -= expr -> var = var - expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_SUB);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " -= <expression>" << std::endl;
        
    } else if (assignmentText.find("*=") != std::string::npos) {
        // Compound multiplication: var *= expr -> var = var * expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_MUL);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " *= <expression>" << std::endl;
        
    } else if (assignmentText.find("/=") != std::string::npos) {
        // Compound division: var /= expr -> var = var / expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_DIV);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " /= <expression>" << std::endl;
        
    } else if (assignmentText.find("%=") != std::string::npos) {
        // Compound modulo: var %= expr -> var = var % expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_MOD);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " %= <expression>" << std::endl;
        
    } else if (assignmentText.find("&=") != std::string::npos) {
        // Compound bitwise AND: var &= expr -> var = var & expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_BITWISE_AND);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " &= <expression>" << std::endl;
        
    } else if (assignmentText.find("|=") != std::string::npos) {
        // Compound bitwise OR: var |= expr -> var = var | expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_BITWISE_OR);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " |= <expression>" << std::endl;
        
    } else if (assignmentText.find("^=") != std::string::npos) {
        // Compound bitwise XOR: var ^= expr -> var = var ^ expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_BITWISE_XOR);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " ^= <expression>" << std::endl;
        
    } else if (assignmentText.find("<<=") != std::string::npos) {
        // Compound left shift: var <<= expr -> var = var << expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_SHIFT_LEFT);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " <<= <expression>" << std::endl;
        
    } else if (assignmentText.find(">>=") != std::string::npos) {
        // Compound right shift: var >>= expr -> var = var >> expr
        emitLoadVariable(varName);
        visit(expression);
        emitInstruction(VMOpcode::OP_SHIFT_RIGHT);
        emitStoreVariable(varName);
        std::cout << "Generated compound assignment: " << varName << " >>= <expression>" << std::endl;
        
    } else {
        // Regular assignment: var = expr
        visit(expression);
        emitStoreVariable(varName);
        std::cout << "Generated assignment: " << varName << " = <expression>" << std::endl;
    }
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitFunctionCall(ArduinoCParser::FunctionCallContext *ctx) {
    std::string funcName = ctx->IDENTIFIER()->getText();
    
    // Special handling for printf - requires argument count and string index processing
    if (funcName == "printf") {
        int arg_count = 0;
        int string_index = 0;
        
        // Process arguments and count them
        if (ctx->argumentList()) {
            auto args = ctx->argumentList()->expression();
            arg_count = args.size();
            
            // First argument should be string literal - extract and add to string table
            if (arg_count > 0) {
                auto first_arg = args[0];
                // Check if it's a string literal by examining the parse tree
                std::string arg_text = first_arg->getText();
                if (arg_text.front() == '"' && arg_text.back() == '"') {
                    // Remove quotes and add to string table
                    std::string str_content = arg_text.substr(1, arg_text.length() - 2);
                    string_index = addStringLiteral(str_content);
                    
                    // Process remaining arguments (skip first string literal)
                    for (size_t i = 1; i < args.size(); i++) {
                        visit(args[i]);
                    }
                    arg_count--; // String doesn't go on stack, only other args
                } else {
                    // Not a string literal, process all args normally
                    for (auto arg : args) {
                        visit(arg);
                    }
                }
            }
        }
        
        // Push argument count to stack
        emitPushConstant(arg_count);
        
        // Emit printf with string index in immediate field
        emitInstruction(VMOpcode::OP_PRINTF, static_cast<uint16_t>(string_index));
        std::cout << "Generated printf call: " << arg_count << " args, string_index=" << string_index << std::endl;
        
        return nullptr;
    }
    
    // Regular function call processing
    // Special handling for delay function - convert milliseconds to nanoseconds
    if (funcName == "delay") {
        if (ctx->argumentList()) {
            auto args = ctx->argumentList()->expression();
            if (args.size() == 1) {
                // Visit the argument to get the millisecond value
                visit(args[0]);
                // Convert milliseconds to nanoseconds by multiplying by 1,000,000
                emitInstruction(VMOpcode::OP_PUSH, 0, 1000000);
                emitInstruction(VMOpcode::OP_MUL);
            }
        }
    } else {
        // Process arguments first (push them onto stack in reverse order for correct parameter order)
        if (ctx->argumentList()) {
            auto args = ctx->argumentList()->expression();
            for (auto arg : args) {
                visit(arg);
            }
        }
    }
    
    // Use our unified function call system
    emitFunctionCall(funcName);
    
    return nullptr;
}

antlrcpp::Any BytecodeVisitor::visitExpression(ArduinoCParser::ExpressionContext *ctx) {
    // Handle both assignment and ternary expressions as per grammar
    if (ctx->assignment()) {
        return visit(ctx->assignment());
    } else if (ctx->ternaryExpression()) {
        return visit(ctx->ternaryExpression());
    }
    return nullptr;
}

void BytecodeVisitor::printBytecode() const {
    std::cout << "\nGenerated Bytecode:\n";
    for (size_t i = 0; i < bytecode.size(); ++i) {
        const auto& instr = bytecode[i];
        std::cout << i << ": opcode=0x" << std::hex 
                  << static_cast<int>(instr.opcode) 
                  << " flags=0x" << static_cast<int>(instr.flags)
                  << " immediate=" << std::dec << instr.immediate
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
        
        // Check if offset fits in signed 16-bit range
        if (offset < -32768 || offset > 32767) {
            reportError("Jump offset out of range (-32768 to 32767): " + std::to_string(offset));
            continue;
        }
        
        // Patch the jump instruction
        bytecode[jump_instruction_index].immediate = static_cast<uint16_t>(static_cast<int16_t>(offset));
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
        
        // Check if offset fits in 16-bit range
        if (function_address > 65535) {
            reportError("Function address out of range (0-65535): " + std::to_string(function_address));
            continue;
        }
        
        // Patch the function call instruction
        bytecode[placeholder.instruction_index].immediate = static_cast<uint16_t>(function_address);
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

antlrcpp::Any BytecodeVisitor::visitPrimaryExpression(ArduinoCParser::PrimaryExpressionContext *ctx) {
    if (ctx->functionCall()) {
        return visit(ctx->functionCall());
    } else if (ctx->IDENTIFIER()) {
        std::string varName = ctx->IDENTIFIER()->getText();
        
        // Check if this is array access: IDENTIFIER '[' expression ']'
        if (ctx->expression()) {
            // Array access: arr[index]
            Symbol* symbol = symbolTable.lookupSymbol(varName);
            if (!symbol) {
                reportError("Undefined array: " + varName);
                return nullptr;
            }
            
            if (symbol->symbolType != SymbolType::ARRAY) {
                reportError("Variable is not an array: " + varName);
                return nullptr;
            }
            
            // Evaluate index expression
            visit(ctx->expression());
            
            // Emit array load instruction
            emitInstruction(VMOpcode::OP_LOAD_ARRAY, static_cast<uint16_t>(symbol->arrayId));
        } else {
            // Load variable value onto stack
            emitLoadVariable(varName);
        }
    } else if (ctx->INTEGER()) {
        // Check if this is a negative number
        if (ctx->children.size() == 2 && ctx->children[0]->getText() == "-") {
            // Negative number: '-' INTEGER
            int value = -std::stoi(ctx->INTEGER()->getText());
            emitPushConstant(value);
        } else {
            // Regular positive integer
            int value = std::stoi(ctx->INTEGER()->getText());
            emitPushConstant(value);
        }
    } else if (ctx->STRING()) {
        // Handle string literal
        std::string str = ctx->STRING()->getText();
        // Remove quotes
        str = str.substr(1, str.length() - 2);
        int stringIndex = addStringLiteral(str);
        emitPushConstant(stringIndex);
    } else if (ctx->expression()) {
        // Parenthesized expression
        return visit(ctx->expression());
    }
    return nullptr;
}