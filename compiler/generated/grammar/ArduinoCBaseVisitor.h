
// Generated from grammar/ArduinoC.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ArduinoCVisitor.h"


/**
 * This class provides an empty implementation of ArduinoCVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ArduinoCBaseVisitor : public ArduinoCVisitor {
public:

  virtual std::any visitProgram(ArduinoCParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclaration(ArduinoCParser::DeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionDefinition(ArduinoCParser::FunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterList(ArduinoCParser::ParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameter(ArduinoCParser::ParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompoundStatement(ArduinoCParser::CompoundStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(ArduinoCParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStatement(ArduinoCParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStatement(ArduinoCParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStatement(ArduinoCParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionStatement(ArduinoCParser::ExpressionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(ArduinoCParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTernaryExpression(ArduinoCParser::TernaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpression(ArduinoCParser::LogicalOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpression(ArduinoCParser::LogicalAndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalNotExpression(ArduinoCParser::LogicalNotExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitwiseOrExpression(ArduinoCParser::BitwiseOrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitwiseXorExpression(ArduinoCParser::BitwiseXorExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitwiseAndExpression(ArduinoCParser::BitwiseAndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalExpression(ArduinoCParser::ConditionalExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpression(ArduinoCParser::ShiftExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArithmeticExpression(ArduinoCParser::ArithmeticExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(ArduinoCParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpression(ArduinoCParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComparisonOperator(ArduinoCParser::ComparisonOperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignment(ArduinoCParser::AssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCall(ArduinoCParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgumentList(ArduinoCParser::ArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(ArduinoCParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }


};

