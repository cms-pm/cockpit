
// Generated from grammar/ArduinoC.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ArduinoCParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ArduinoCParser.
 */
class  ArduinoCVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ArduinoCParser.
   */
    virtual std::any visitProgram(ArduinoCParser::ProgramContext *context) = 0;

    virtual std::any visitDeclaration(ArduinoCParser::DeclarationContext *context) = 0;

    virtual std::any visitFunctionDefinition(ArduinoCParser::FunctionDefinitionContext *context) = 0;

    virtual std::any visitParameterList(ArduinoCParser::ParameterListContext *context) = 0;

    virtual std::any visitParameter(ArduinoCParser::ParameterContext *context) = 0;

    virtual std::any visitCompoundStatement(ArduinoCParser::CompoundStatementContext *context) = 0;

    virtual std::any visitStatement(ArduinoCParser::StatementContext *context) = 0;

    virtual std::any visitReturnStatement(ArduinoCParser::ReturnStatementContext *context) = 0;

    virtual std::any visitIfStatement(ArduinoCParser::IfStatementContext *context) = 0;

    virtual std::any visitWhileStatement(ArduinoCParser::WhileStatementContext *context) = 0;

    virtual std::any visitExpressionStatement(ArduinoCParser::ExpressionStatementContext *context) = 0;

    virtual std::any visitExpression(ArduinoCParser::ExpressionContext *context) = 0;

    virtual std::any visitTernaryExpression(ArduinoCParser::TernaryExpressionContext *context) = 0;

    virtual std::any visitLogicalOrExpression(ArduinoCParser::LogicalOrExpressionContext *context) = 0;

    virtual std::any visitLogicalAndExpression(ArduinoCParser::LogicalAndExpressionContext *context) = 0;

    virtual std::any visitLogicalNotExpression(ArduinoCParser::LogicalNotExpressionContext *context) = 0;

    virtual std::any visitBitwiseOrExpression(ArduinoCParser::BitwiseOrExpressionContext *context) = 0;

    virtual std::any visitBitwiseXorExpression(ArduinoCParser::BitwiseXorExpressionContext *context) = 0;

    virtual std::any visitBitwiseAndExpression(ArduinoCParser::BitwiseAndExpressionContext *context) = 0;

    virtual std::any visitConditionalExpression(ArduinoCParser::ConditionalExpressionContext *context) = 0;

    virtual std::any visitShiftExpression(ArduinoCParser::ShiftExpressionContext *context) = 0;

    virtual std::any visitArithmeticExpression(ArduinoCParser::ArithmeticExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(ArduinoCParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitPrimaryExpression(ArduinoCParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitComparisonOperator(ArduinoCParser::ComparisonOperatorContext *context) = 0;

    virtual std::any visitAssignment(ArduinoCParser::AssignmentContext *context) = 0;

    virtual std::any visitFunctionCall(ArduinoCParser::FunctionCallContext *context) = 0;

    virtual std::any visitArgumentList(ArduinoCParser::ArgumentListContext *context) = 0;

    virtual std::any visitType(ArduinoCParser::TypeContext *context) = 0;


};

