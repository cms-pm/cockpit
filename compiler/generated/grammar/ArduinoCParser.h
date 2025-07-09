
// Generated from grammar/ArduinoC.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ArduinoCParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    T__44 = 45, QUESTION_MARK = 46, COLON = 47, IDENTIFIER = 48, INTEGER = 49, 
    STRING = 50, WS = 51, COMMENT = 52, BLOCK_COMMENT = 53
  };

  enum {
    RuleProgram = 0, RuleDeclaration = 1, RuleFunctionDefinition = 2, RuleParameterList = 3, 
    RuleParameter = 4, RuleCompoundStatement = 5, RuleStatement = 6, RuleReturnStatement = 7, 
    RuleIfStatement = 8, RuleWhileStatement = 9, RuleExpressionStatement = 10, 
    RuleExpression = 11, RuleTernaryExpression = 12, RuleLogicalOrExpression = 13, 
    RuleLogicalAndExpression = 14, RuleLogicalNotExpression = 15, RuleBitwiseOrExpression = 16, 
    RuleBitwiseXorExpression = 17, RuleBitwiseAndExpression = 18, RuleConditionalExpression = 19, 
    RuleShiftExpression = 20, RuleArithmeticExpression = 21, RuleMultiplicativeExpression = 22, 
    RulePrimaryExpression = 23, RuleComparisonOperator = 24, RuleAssignment = 25, 
    RuleFunctionCall = 26, RuleArgumentList = 27, RuleType = 28
  };

  explicit ArduinoCParser(antlr4::TokenStream *input);

  ArduinoCParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ArduinoCParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ProgramContext;
  class DeclarationContext;
  class FunctionDefinitionContext;
  class ParameterListContext;
  class ParameterContext;
  class CompoundStatementContext;
  class StatementContext;
  class ReturnStatementContext;
  class IfStatementContext;
  class WhileStatementContext;
  class ExpressionStatementContext;
  class ExpressionContext;
  class TernaryExpressionContext;
  class LogicalOrExpressionContext;
  class LogicalAndExpressionContext;
  class LogicalNotExpressionContext;
  class BitwiseOrExpressionContext;
  class BitwiseXorExpressionContext;
  class BitwiseAndExpressionContext;
  class ConditionalExpressionContext;
  class ShiftExpressionContext;
  class ArithmeticExpressionContext;
  class MultiplicativeExpressionContext;
  class PrimaryExpressionContext;
  class ComparisonOperatorContext;
  class AssignmentContext;
  class FunctionCallContext;
  class ArgumentListContext;
  class TypeContext; 

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<DeclarationContext *> declaration();
    DeclarationContext* declaration(size_t i);
    std::vector<FunctionDefinitionContext *> functionDefinition();
    FunctionDefinitionContext* functionDefinition(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramContext* program();

  class  DeclarationContext : public antlr4::ParserRuleContext {
  public:
    DeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *IDENTIFIER();
    antlr4::tree::TerminalNode *INTEGER();
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DeclarationContext* declaration();

  class  FunctionDefinitionContext : public antlr4::ParserRuleContext {
  public:
    FunctionDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *IDENTIFIER();
    CompoundStatementContext *compoundStatement();
    ParameterListContext *parameterList();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionDefinitionContext* functionDefinition();

  class  ParameterListContext : public antlr4::ParserRuleContext {
  public:
    ParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ParameterContext *> parameter();
    ParameterContext* parameter(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterListContext* parameterList();

  class  ParameterContext : public antlr4::ParserRuleContext {
  public:
    ParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *IDENTIFIER();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterContext* parameter();

  class  CompoundStatementContext : public antlr4::ParserRuleContext {
  public:
    CompoundStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CompoundStatementContext* compoundStatement();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionStatementContext *expressionStatement();
    CompoundStatementContext *compoundStatement();
    DeclarationContext *declaration();
    IfStatementContext *ifStatement();
    WhileStatementContext *whileStatement();
    ReturnStatementContext *returnStatement();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  ReturnStatementContext : public antlr4::ParserRuleContext {
  public:
    ReturnStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnStatementContext* returnStatement();

  class  IfStatementContext : public antlr4::ParserRuleContext {
  public:
    IfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IfStatementContext* ifStatement();

  class  WhileStatementContext : public antlr4::ParserRuleContext {
  public:
    WhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    StatementContext *statement();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhileStatementContext* whileStatement();

  class  ExpressionStatementContext : public antlr4::ParserRuleContext {
  public:
    ExpressionStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionStatementContext* expressionStatement();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AssignmentContext *assignment();
    TernaryExpressionContext *ternaryExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();

  class  TernaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    TernaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LogicalOrExpressionContext *logicalOrExpression();
    antlr4::tree::TerminalNode *QUESTION_MARK();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *COLON();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TernaryExpressionContext* ternaryExpression();

  class  LogicalOrExpressionContext : public antlr4::ParserRuleContext {
  public:
    LogicalOrExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<LogicalAndExpressionContext *> logicalAndExpression();
    LogicalAndExpressionContext* logicalAndExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LogicalOrExpressionContext* logicalOrExpression();

  class  LogicalAndExpressionContext : public antlr4::ParserRuleContext {
  public:
    LogicalAndExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<LogicalNotExpressionContext *> logicalNotExpression();
    LogicalNotExpressionContext* logicalNotExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LogicalAndExpressionContext* logicalAndExpression();

  class  LogicalNotExpressionContext : public antlr4::ParserRuleContext {
  public:
    LogicalNotExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LogicalNotExpressionContext *logicalNotExpression();
    BitwiseOrExpressionContext *bitwiseOrExpression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LogicalNotExpressionContext* logicalNotExpression();

  class  BitwiseOrExpressionContext : public antlr4::ParserRuleContext {
  public:
    BitwiseOrExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitwiseXorExpressionContext *> bitwiseXorExpression();
    BitwiseXorExpressionContext* bitwiseXorExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseOrExpressionContext* bitwiseOrExpression();

  class  BitwiseXorExpressionContext : public antlr4::ParserRuleContext {
  public:
    BitwiseXorExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<BitwiseAndExpressionContext *> bitwiseAndExpression();
    BitwiseAndExpressionContext* bitwiseAndExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseXorExpressionContext* bitwiseXorExpression();

  class  BitwiseAndExpressionContext : public antlr4::ParserRuleContext {
  public:
    BitwiseAndExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ConditionalExpressionContext *> conditionalExpression();
    ConditionalExpressionContext* conditionalExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BitwiseAndExpressionContext* bitwiseAndExpression();

  class  ConditionalExpressionContext : public antlr4::ParserRuleContext {
  public:
    ConditionalExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ShiftExpressionContext *> shiftExpression();
    ShiftExpressionContext* shiftExpression(size_t i);
    ComparisonOperatorContext *comparisonOperator();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionalExpressionContext* conditionalExpression();

  class  ShiftExpressionContext : public antlr4::ParserRuleContext {
  public:
    ShiftExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ArithmeticExpressionContext *> arithmeticExpression();
    ArithmeticExpressionContext* arithmeticExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ShiftExpressionContext* shiftExpression();

  class  ArithmeticExpressionContext : public antlr4::ParserRuleContext {
  public:
    ArithmeticExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<MultiplicativeExpressionContext *> multiplicativeExpression();
    MultiplicativeExpressionContext* multiplicativeExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArithmeticExpressionContext* arithmeticExpression();

  class  MultiplicativeExpressionContext : public antlr4::ParserRuleContext {
  public:
    MultiplicativeExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<PrimaryExpressionContext *> primaryExpression();
    PrimaryExpressionContext* primaryExpression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MultiplicativeExpressionContext* multiplicativeExpression();

  class  PrimaryExpressionContext : public antlr4::ParserRuleContext {
  public:
    PrimaryExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    FunctionCallContext *functionCall();
    antlr4::tree::TerminalNode *IDENTIFIER();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *INTEGER();
    antlr4::tree::TerminalNode *STRING();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryExpressionContext* primaryExpression();

  class  ComparisonOperatorContext : public antlr4::ParserRuleContext {
  public:
    ComparisonOperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComparisonOperatorContext* comparisonOperator();

  class  AssignmentContext : public antlr4::ParserRuleContext {
  public:
    AssignmentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentContext* assignment();

  class  FunctionCallContext : public antlr4::ParserRuleContext {
  public:
    FunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IDENTIFIER();
    ArgumentListContext *argumentList();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionCallContext* functionCall();

  class  ArgumentListContext : public antlr4::ParserRuleContext {
  public:
    ArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgumentListContext* argumentList();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

