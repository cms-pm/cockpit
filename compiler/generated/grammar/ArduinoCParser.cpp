
// Generated from grammar/ArduinoC.g4 by ANTLR 4.13.2


#include "ArduinoCVisitor.h"

#include "ArduinoCParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct ArduinoCParserStaticData final {
  ArduinoCParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ArduinoCParserStaticData(const ArduinoCParserStaticData&) = delete;
  ArduinoCParserStaticData(ArduinoCParserStaticData&&) = delete;
  ArduinoCParserStaticData& operator=(const ArduinoCParserStaticData&) = delete;
  ArduinoCParserStaticData& operator=(ArduinoCParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag arduinocParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ArduinoCParserStaticData> arduinocParserStaticData = nullptr;

void arduinocParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (arduinocParserStaticData != nullptr) {
    return;
  }
#else
  assert(arduinocParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ArduinoCParserStaticData>(
    std::vector<std::string>{
      "program", "declaration", "functionDefinition", "parameterList", "parameter", 
      "compoundStatement", "statement", "returnStatement", "ifStatement", 
      "whileStatement", "expressionStatement", "expression", "ternaryExpression", 
      "logicalOrExpression", "logicalAndExpression", "logicalNotExpression", 
      "bitwiseOrExpression", "bitwiseXorExpression", "bitwiseAndExpression", 
      "conditionalExpression", "shiftExpression", "arithmeticExpression", 
      "multiplicativeExpression", "primaryExpression", "comparisonOperator", 
      "assignment", "functionCall", "argumentList", "type"
    },
    std::vector<std::string>{
      "", "'['", "']'", "'='", "';'", "'('", "')'", "','", "'{'", "'}'", 
      "'return'", "'if'", "'else'", "'while'", "'||'", "'&&'", "'!'", "'~'", 
      "'|'", "'^'", "'&'", "'<<'", "'>>'", "'+'", "'-'", "'*'", "'/'", "'%'", 
      "'=='", "'!='", "'<'", "'>'", "'<='", "'>='", "'+='", "'-='", "'*='", 
      "'/='", "'%='", "'&='", "'|='", "'^='", "'<<='", "'>>='", "'int'", 
      "'void'", "'\\u003F'", "':'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "QUESTION_MARK", "COLON", 
      "IDENTIFIER", "INTEGER", "STRING", "WS", "COMMENT", "BLOCK_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,53,312,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,1,0,1,0,5,0,61,8,0,10,0,12,0,64,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,3,
  	1,73,8,1,1,1,1,1,3,1,77,8,1,1,1,1,1,1,2,1,2,1,2,1,2,3,2,85,8,2,1,2,1,
  	2,1,2,1,3,1,3,1,3,5,3,93,8,3,10,3,12,3,96,9,3,1,4,1,4,1,4,1,5,1,5,5,5,
  	103,8,5,10,5,12,5,106,9,5,1,5,1,5,1,6,1,6,1,6,1,6,1,6,1,6,3,6,116,8,6,
  	1,7,1,7,3,7,120,8,7,1,7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,3,8,131,8,8,1,
  	9,1,9,1,9,1,9,1,9,1,9,1,10,3,10,140,8,10,1,10,1,10,1,11,1,11,3,11,146,
  	8,11,1,12,1,12,1,12,1,12,1,12,1,12,3,12,154,8,12,1,13,1,13,1,13,5,13,
  	159,8,13,10,13,12,13,162,9,13,1,14,1,14,1,14,5,14,167,8,14,10,14,12,14,
  	170,9,14,1,15,1,15,1,15,1,15,1,15,3,15,177,8,15,1,16,1,16,1,16,5,16,182,
  	8,16,10,16,12,16,185,9,16,1,17,1,17,1,17,5,17,190,8,17,10,17,12,17,193,
  	9,17,1,18,1,18,1,18,5,18,198,8,18,10,18,12,18,201,9,18,1,19,1,19,1,19,
  	1,19,3,19,207,8,19,1,20,1,20,1,20,5,20,212,8,20,10,20,12,20,215,9,20,
  	1,21,1,21,1,21,5,21,220,8,21,10,21,12,21,223,9,21,1,22,1,22,1,22,5,22,
  	228,8,22,10,22,12,22,231,9,22,1,23,1,23,1,23,1,23,1,23,1,23,3,23,239,
  	8,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,3,23,249,8,23,1,24,1,24,
  	1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,
  	1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,
  	1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,3,25,293,
  	8,25,1,26,1,26,1,26,3,26,298,8,26,1,26,1,26,1,27,1,27,1,27,5,27,305,8,
  	27,10,27,12,27,308,9,27,1,28,1,28,1,28,0,0,29,0,2,4,6,8,10,12,14,16,18,
  	20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,0,5,1,0,21,22,
  	1,0,23,24,1,0,25,27,1,0,28,33,1,0,44,45,329,0,62,1,0,0,0,2,67,1,0,0,0,
  	4,80,1,0,0,0,6,89,1,0,0,0,8,97,1,0,0,0,10,100,1,0,0,0,12,115,1,0,0,0,
  	14,117,1,0,0,0,16,123,1,0,0,0,18,132,1,0,0,0,20,139,1,0,0,0,22,145,1,
  	0,0,0,24,147,1,0,0,0,26,155,1,0,0,0,28,163,1,0,0,0,30,176,1,0,0,0,32,
  	178,1,0,0,0,34,186,1,0,0,0,36,194,1,0,0,0,38,202,1,0,0,0,40,208,1,0,0,
  	0,42,216,1,0,0,0,44,224,1,0,0,0,46,248,1,0,0,0,48,250,1,0,0,0,50,292,
  	1,0,0,0,52,294,1,0,0,0,54,301,1,0,0,0,56,309,1,0,0,0,58,61,3,2,1,0,59,
  	61,3,4,2,0,60,58,1,0,0,0,60,59,1,0,0,0,61,64,1,0,0,0,62,60,1,0,0,0,62,
  	63,1,0,0,0,63,65,1,0,0,0,64,62,1,0,0,0,65,66,5,0,0,1,66,1,1,0,0,0,67,
  	68,3,56,28,0,68,72,5,48,0,0,69,70,5,1,0,0,70,71,5,49,0,0,71,73,5,2,0,
  	0,72,69,1,0,0,0,72,73,1,0,0,0,73,76,1,0,0,0,74,75,5,3,0,0,75,77,3,22,
  	11,0,76,74,1,0,0,0,76,77,1,0,0,0,77,78,1,0,0,0,78,79,5,4,0,0,79,3,1,0,
  	0,0,80,81,3,56,28,0,81,82,5,48,0,0,82,84,5,5,0,0,83,85,3,6,3,0,84,83,
  	1,0,0,0,84,85,1,0,0,0,85,86,1,0,0,0,86,87,5,6,0,0,87,88,3,10,5,0,88,5,
  	1,0,0,0,89,94,3,8,4,0,90,91,5,7,0,0,91,93,3,8,4,0,92,90,1,0,0,0,93,96,
  	1,0,0,0,94,92,1,0,0,0,94,95,1,0,0,0,95,7,1,0,0,0,96,94,1,0,0,0,97,98,
  	3,56,28,0,98,99,5,48,0,0,99,9,1,0,0,0,100,104,5,8,0,0,101,103,3,12,6,
  	0,102,101,1,0,0,0,103,106,1,0,0,0,104,102,1,0,0,0,104,105,1,0,0,0,105,
  	107,1,0,0,0,106,104,1,0,0,0,107,108,5,9,0,0,108,11,1,0,0,0,109,116,3,
  	20,10,0,110,116,3,10,5,0,111,116,3,2,1,0,112,116,3,16,8,0,113,116,3,18,
  	9,0,114,116,3,14,7,0,115,109,1,0,0,0,115,110,1,0,0,0,115,111,1,0,0,0,
  	115,112,1,0,0,0,115,113,1,0,0,0,115,114,1,0,0,0,116,13,1,0,0,0,117,119,
  	5,10,0,0,118,120,3,22,11,0,119,118,1,0,0,0,119,120,1,0,0,0,120,121,1,
  	0,0,0,121,122,5,4,0,0,122,15,1,0,0,0,123,124,5,11,0,0,124,125,5,5,0,0,
  	125,126,3,22,11,0,126,127,5,6,0,0,127,130,3,12,6,0,128,129,5,12,0,0,129,
  	131,3,12,6,0,130,128,1,0,0,0,130,131,1,0,0,0,131,17,1,0,0,0,132,133,5,
  	13,0,0,133,134,5,5,0,0,134,135,3,22,11,0,135,136,5,6,0,0,136,137,3,12,
  	6,0,137,19,1,0,0,0,138,140,3,22,11,0,139,138,1,0,0,0,139,140,1,0,0,0,
  	140,141,1,0,0,0,141,142,5,4,0,0,142,21,1,0,0,0,143,146,3,50,25,0,144,
  	146,3,24,12,0,145,143,1,0,0,0,145,144,1,0,0,0,146,23,1,0,0,0,147,153,
  	3,26,13,0,148,149,5,46,0,0,149,150,3,22,11,0,150,151,5,47,0,0,151,152,
  	3,22,11,0,152,154,1,0,0,0,153,148,1,0,0,0,153,154,1,0,0,0,154,25,1,0,
  	0,0,155,160,3,28,14,0,156,157,5,14,0,0,157,159,3,28,14,0,158,156,1,0,
  	0,0,159,162,1,0,0,0,160,158,1,0,0,0,160,161,1,0,0,0,161,27,1,0,0,0,162,
  	160,1,0,0,0,163,168,3,30,15,0,164,165,5,15,0,0,165,167,3,30,15,0,166,
  	164,1,0,0,0,167,170,1,0,0,0,168,166,1,0,0,0,168,169,1,0,0,0,169,29,1,
  	0,0,0,170,168,1,0,0,0,171,172,5,16,0,0,172,177,3,30,15,0,173,174,5,17,
  	0,0,174,177,3,30,15,0,175,177,3,32,16,0,176,171,1,0,0,0,176,173,1,0,0,
  	0,176,175,1,0,0,0,177,31,1,0,0,0,178,183,3,34,17,0,179,180,5,18,0,0,180,
  	182,3,34,17,0,181,179,1,0,0,0,182,185,1,0,0,0,183,181,1,0,0,0,183,184,
  	1,0,0,0,184,33,1,0,0,0,185,183,1,0,0,0,186,191,3,36,18,0,187,188,5,19,
  	0,0,188,190,3,36,18,0,189,187,1,0,0,0,190,193,1,0,0,0,191,189,1,0,0,0,
  	191,192,1,0,0,0,192,35,1,0,0,0,193,191,1,0,0,0,194,199,3,38,19,0,195,
  	196,5,20,0,0,196,198,3,38,19,0,197,195,1,0,0,0,198,201,1,0,0,0,199,197,
  	1,0,0,0,199,200,1,0,0,0,200,37,1,0,0,0,201,199,1,0,0,0,202,206,3,40,20,
  	0,203,204,3,48,24,0,204,205,3,40,20,0,205,207,1,0,0,0,206,203,1,0,0,0,
  	206,207,1,0,0,0,207,39,1,0,0,0,208,213,3,42,21,0,209,210,7,0,0,0,210,
  	212,3,42,21,0,211,209,1,0,0,0,212,215,1,0,0,0,213,211,1,0,0,0,213,214,
  	1,0,0,0,214,41,1,0,0,0,215,213,1,0,0,0,216,221,3,44,22,0,217,218,7,1,
  	0,0,218,220,3,44,22,0,219,217,1,0,0,0,220,223,1,0,0,0,221,219,1,0,0,0,
  	221,222,1,0,0,0,222,43,1,0,0,0,223,221,1,0,0,0,224,229,3,46,23,0,225,
  	226,7,2,0,0,226,228,3,46,23,0,227,225,1,0,0,0,228,231,1,0,0,0,229,227,
  	1,0,0,0,229,230,1,0,0,0,230,45,1,0,0,0,231,229,1,0,0,0,232,249,3,52,26,
  	0,233,238,5,48,0,0,234,235,5,1,0,0,235,236,3,22,11,0,236,237,5,2,0,0,
  	237,239,1,0,0,0,238,234,1,0,0,0,238,239,1,0,0,0,239,249,1,0,0,0,240,249,
  	5,49,0,0,241,242,5,24,0,0,242,249,5,49,0,0,243,249,5,50,0,0,244,245,5,
  	5,0,0,245,246,3,22,11,0,246,247,5,6,0,0,247,249,1,0,0,0,248,232,1,0,0,
  	0,248,233,1,0,0,0,248,240,1,0,0,0,248,241,1,0,0,0,248,243,1,0,0,0,248,
  	244,1,0,0,0,249,47,1,0,0,0,250,251,7,3,0,0,251,49,1,0,0,0,252,253,5,48,
  	0,0,253,254,5,3,0,0,254,293,3,22,11,0,255,256,5,48,0,0,256,257,5,1,0,
  	0,257,258,3,22,11,0,258,259,5,2,0,0,259,260,5,3,0,0,260,261,3,22,11,0,
  	261,293,1,0,0,0,262,263,5,48,0,0,263,264,5,34,0,0,264,293,3,22,11,0,265,
  	266,5,48,0,0,266,267,5,35,0,0,267,293,3,22,11,0,268,269,5,48,0,0,269,
  	270,5,36,0,0,270,293,3,22,11,0,271,272,5,48,0,0,272,273,5,37,0,0,273,
  	293,3,22,11,0,274,275,5,48,0,0,275,276,5,38,0,0,276,293,3,22,11,0,277,
  	278,5,48,0,0,278,279,5,39,0,0,279,293,3,22,11,0,280,281,5,48,0,0,281,
  	282,5,40,0,0,282,293,3,22,11,0,283,284,5,48,0,0,284,285,5,41,0,0,285,
  	293,3,22,11,0,286,287,5,48,0,0,287,288,5,42,0,0,288,293,3,22,11,0,289,
  	290,5,48,0,0,290,291,5,43,0,0,291,293,3,22,11,0,292,252,1,0,0,0,292,255,
  	1,0,0,0,292,262,1,0,0,0,292,265,1,0,0,0,292,268,1,0,0,0,292,271,1,0,0,
  	0,292,274,1,0,0,0,292,277,1,0,0,0,292,280,1,0,0,0,292,283,1,0,0,0,292,
  	286,1,0,0,0,292,289,1,0,0,0,293,51,1,0,0,0,294,295,5,48,0,0,295,297,5,
  	5,0,0,296,298,3,54,27,0,297,296,1,0,0,0,297,298,1,0,0,0,298,299,1,0,0,
  	0,299,300,5,6,0,0,300,53,1,0,0,0,301,306,3,22,11,0,302,303,5,7,0,0,303,
  	305,3,22,11,0,304,302,1,0,0,0,305,308,1,0,0,0,306,304,1,0,0,0,306,307,
  	1,0,0,0,307,55,1,0,0,0,308,306,1,0,0,0,309,310,7,4,0,0,310,57,1,0,0,0,
  	28,60,62,72,76,84,94,104,115,119,130,139,145,153,160,168,176,183,191,
  	199,206,213,221,229,238,248,292,297,306
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  arduinocParserStaticData = std::move(staticData);
}

}

ArduinoCParser::ArduinoCParser(TokenStream *input) : ArduinoCParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ArduinoCParser::ArduinoCParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ArduinoCParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *arduinocParserStaticData->atn, arduinocParserStaticData->decisionToDFA, arduinocParserStaticData->sharedContextCache, options);
}

ArduinoCParser::~ArduinoCParser() {
  delete _interpreter;
}

const atn::ATN& ArduinoCParser::getATN() const {
  return *arduinocParserStaticData->atn;
}

std::string ArduinoCParser::getGrammarFileName() const {
  return "ArduinoC.g4";
}

const std::vector<std::string>& ArduinoCParser::getRuleNames() const {
  return arduinocParserStaticData->ruleNames;
}

const dfa::Vocabulary& ArduinoCParser::getVocabulary() const {
  return arduinocParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ArduinoCParser::getSerializedATN() const {
  return arduinocParserStaticData->serializedATN;
}


//----------------- ProgramContext ------------------------------------------------------------------

ArduinoCParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArduinoCParser::ProgramContext::EOF() {
  return getToken(ArduinoCParser::EOF, 0);
}

std::vector<ArduinoCParser::DeclarationContext *> ArduinoCParser::ProgramContext::declaration() {
  return getRuleContexts<ArduinoCParser::DeclarationContext>();
}

ArduinoCParser::DeclarationContext* ArduinoCParser::ProgramContext::declaration(size_t i) {
  return getRuleContext<ArduinoCParser::DeclarationContext>(i);
}

std::vector<ArduinoCParser::FunctionDefinitionContext *> ArduinoCParser::ProgramContext::functionDefinition() {
  return getRuleContexts<ArduinoCParser::FunctionDefinitionContext>();
}

ArduinoCParser::FunctionDefinitionContext* ArduinoCParser::ProgramContext::functionDefinition(size_t i) {
  return getRuleContext<ArduinoCParser::FunctionDefinitionContext>(i);
}


size_t ArduinoCParser::ProgramContext::getRuleIndex() const {
  return ArduinoCParser::RuleProgram;
}


std::any ArduinoCParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ProgramContext* ArduinoCParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 0, ArduinoCParser::RuleProgram);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(62);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__43

    || _la == ArduinoCParser::T__44) {
      setState(60);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(58);
        declaration();
        break;
      }

      case 2: {
        setState(59);
        functionDefinition();
        break;
      }

      default:
        break;
      }
      setState(64);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(65);
    match(ArduinoCParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclarationContext ------------------------------------------------------------------

ArduinoCParser::DeclarationContext::DeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::TypeContext* ArduinoCParser::DeclarationContext::type() {
  return getRuleContext<ArduinoCParser::TypeContext>(0);
}

tree::TerminalNode* ArduinoCParser::DeclarationContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}

tree::TerminalNode* ArduinoCParser::DeclarationContext::INTEGER() {
  return getToken(ArduinoCParser::INTEGER, 0);
}

ArduinoCParser::ExpressionContext* ArduinoCParser::DeclarationContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}


size_t ArduinoCParser::DeclarationContext::getRuleIndex() const {
  return ArduinoCParser::RuleDeclaration;
}


std::any ArduinoCParser::DeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitDeclaration(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::DeclarationContext* ArduinoCParser::declaration() {
  DeclarationContext *_localctx = _tracker.createInstance<DeclarationContext>(_ctx, getState());
  enterRule(_localctx, 2, ArduinoCParser::RuleDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(67);
    type();
    setState(68);
    match(ArduinoCParser::IDENTIFIER);
    setState(72);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArduinoCParser::T__0) {
      setState(69);
      match(ArduinoCParser::T__0);
      setState(70);
      match(ArduinoCParser::INTEGER);
      setState(71);
      match(ArduinoCParser::T__1);
    }
    setState(76);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArduinoCParser::T__2) {
      setState(74);
      match(ArduinoCParser::T__2);
      setState(75);
      expression();
    }
    setState(78);
    match(ArduinoCParser::T__3);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionDefinitionContext ------------------------------------------------------------------

ArduinoCParser::FunctionDefinitionContext::FunctionDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::TypeContext* ArduinoCParser::FunctionDefinitionContext::type() {
  return getRuleContext<ArduinoCParser::TypeContext>(0);
}

tree::TerminalNode* ArduinoCParser::FunctionDefinitionContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}

ArduinoCParser::CompoundStatementContext* ArduinoCParser::FunctionDefinitionContext::compoundStatement() {
  return getRuleContext<ArduinoCParser::CompoundStatementContext>(0);
}

ArduinoCParser::ParameterListContext* ArduinoCParser::FunctionDefinitionContext::parameterList() {
  return getRuleContext<ArduinoCParser::ParameterListContext>(0);
}


size_t ArduinoCParser::FunctionDefinitionContext::getRuleIndex() const {
  return ArduinoCParser::RuleFunctionDefinition;
}


std::any ArduinoCParser::FunctionDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitFunctionDefinition(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::FunctionDefinitionContext* ArduinoCParser::functionDefinition() {
  FunctionDefinitionContext *_localctx = _tracker.createInstance<FunctionDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 4, ArduinoCParser::RuleFunctionDefinition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(80);
    type();
    setState(81);
    match(ArduinoCParser::IDENTIFIER);
    setState(82);
    match(ArduinoCParser::T__4);
    setState(84);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArduinoCParser::T__43

    || _la == ArduinoCParser::T__44) {
      setState(83);
      parameterList();
    }
    setState(86);
    match(ArduinoCParser::T__5);
    setState(87);
    compoundStatement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterListContext ------------------------------------------------------------------

ArduinoCParser::ParameterListContext::ParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::ParameterContext *> ArduinoCParser::ParameterListContext::parameter() {
  return getRuleContexts<ArduinoCParser::ParameterContext>();
}

ArduinoCParser::ParameterContext* ArduinoCParser::ParameterListContext::parameter(size_t i) {
  return getRuleContext<ArduinoCParser::ParameterContext>(i);
}


size_t ArduinoCParser::ParameterListContext::getRuleIndex() const {
  return ArduinoCParser::RuleParameterList;
}


std::any ArduinoCParser::ParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitParameterList(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ParameterListContext* ArduinoCParser::parameterList() {
  ParameterListContext *_localctx = _tracker.createInstance<ParameterListContext>(_ctx, getState());
  enterRule(_localctx, 6, ArduinoCParser::RuleParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(89);
    parameter();
    setState(94);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__6) {
      setState(90);
      match(ArduinoCParser::T__6);
      setState(91);
      parameter();
      setState(96);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterContext ------------------------------------------------------------------

ArduinoCParser::ParameterContext::ParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::TypeContext* ArduinoCParser::ParameterContext::type() {
  return getRuleContext<ArduinoCParser::TypeContext>(0);
}

tree::TerminalNode* ArduinoCParser::ParameterContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}


size_t ArduinoCParser::ParameterContext::getRuleIndex() const {
  return ArduinoCParser::RuleParameter;
}


std::any ArduinoCParser::ParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitParameter(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ParameterContext* ArduinoCParser::parameter() {
  ParameterContext *_localctx = _tracker.createInstance<ParameterContext>(_ctx, getState());
  enterRule(_localctx, 8, ArduinoCParser::RuleParameter);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(97);
    type();
    setState(98);
    match(ArduinoCParser::IDENTIFIER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CompoundStatementContext ------------------------------------------------------------------

ArduinoCParser::CompoundStatementContext::CompoundStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::StatementContext *> ArduinoCParser::CompoundStatementContext::statement() {
  return getRuleContexts<ArduinoCParser::StatementContext>();
}

ArduinoCParser::StatementContext* ArduinoCParser::CompoundStatementContext::statement(size_t i) {
  return getRuleContext<ArduinoCParser::StatementContext>(i);
}


size_t ArduinoCParser::CompoundStatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleCompoundStatement;
}


std::any ArduinoCParser::CompoundStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitCompoundStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::CompoundStatementContext* ArduinoCParser::compoundStatement() {
  CompoundStatementContext *_localctx = _tracker.createInstance<CompoundStatementContext>(_ctx, getState());
  enterRule(_localctx, 10, ArduinoCParser::RuleCompoundStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(100);
    match(ArduinoCParser::T__7);
    setState(104);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2023101412093232) != 0)) {
      setState(101);
      statement();
      setState(106);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(107);
    match(ArduinoCParser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

ArduinoCParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::ExpressionStatementContext* ArduinoCParser::StatementContext::expressionStatement() {
  return getRuleContext<ArduinoCParser::ExpressionStatementContext>(0);
}

ArduinoCParser::CompoundStatementContext* ArduinoCParser::StatementContext::compoundStatement() {
  return getRuleContext<ArduinoCParser::CompoundStatementContext>(0);
}

ArduinoCParser::DeclarationContext* ArduinoCParser::StatementContext::declaration() {
  return getRuleContext<ArduinoCParser::DeclarationContext>(0);
}

ArduinoCParser::IfStatementContext* ArduinoCParser::StatementContext::ifStatement() {
  return getRuleContext<ArduinoCParser::IfStatementContext>(0);
}

ArduinoCParser::WhileStatementContext* ArduinoCParser::StatementContext::whileStatement() {
  return getRuleContext<ArduinoCParser::WhileStatementContext>(0);
}

ArduinoCParser::ReturnStatementContext* ArduinoCParser::StatementContext::returnStatement() {
  return getRuleContext<ArduinoCParser::ReturnStatementContext>(0);
}


size_t ArduinoCParser::StatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleStatement;
}


std::any ArduinoCParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::StatementContext* ArduinoCParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 12, ArduinoCParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(115);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArduinoCParser::T__3:
      case ArduinoCParser::T__4:
      case ArduinoCParser::T__15:
      case ArduinoCParser::T__16:
      case ArduinoCParser::T__23:
      case ArduinoCParser::IDENTIFIER:
      case ArduinoCParser::INTEGER:
      case ArduinoCParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(109);
        expressionStatement();
        break;
      }

      case ArduinoCParser::T__7: {
        enterOuterAlt(_localctx, 2);
        setState(110);
        compoundStatement();
        break;
      }

      case ArduinoCParser::T__43:
      case ArduinoCParser::T__44: {
        enterOuterAlt(_localctx, 3);
        setState(111);
        declaration();
        break;
      }

      case ArduinoCParser::T__10: {
        enterOuterAlt(_localctx, 4);
        setState(112);
        ifStatement();
        break;
      }

      case ArduinoCParser::T__12: {
        enterOuterAlt(_localctx, 5);
        setState(113);
        whileStatement();
        break;
      }

      case ArduinoCParser::T__9: {
        enterOuterAlt(_localctx, 6);
        setState(114);
        returnStatement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnStatementContext ------------------------------------------------------------------

ArduinoCParser::ReturnStatementContext::ReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::ExpressionContext* ArduinoCParser::ReturnStatementContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}


size_t ArduinoCParser::ReturnStatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleReturnStatement;
}


std::any ArduinoCParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ReturnStatementContext* ArduinoCParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 14, ArduinoCParser::RuleReturnStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(117);
    match(ArduinoCParser::T__9);
    setState(119);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324853948448) != 0)) {
      setState(118);
      expression();
    }
    setState(121);
    match(ArduinoCParser::T__3);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IfStatementContext ------------------------------------------------------------------

ArduinoCParser::IfStatementContext::IfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::ExpressionContext* ArduinoCParser::IfStatementContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}

std::vector<ArduinoCParser::StatementContext *> ArduinoCParser::IfStatementContext::statement() {
  return getRuleContexts<ArduinoCParser::StatementContext>();
}

ArduinoCParser::StatementContext* ArduinoCParser::IfStatementContext::statement(size_t i) {
  return getRuleContext<ArduinoCParser::StatementContext>(i);
}


size_t ArduinoCParser::IfStatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleIfStatement;
}


std::any ArduinoCParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::IfStatementContext* ArduinoCParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 16, ArduinoCParser::RuleIfStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(123);
    match(ArduinoCParser::T__10);
    setState(124);
    match(ArduinoCParser::T__4);
    setState(125);
    expression();
    setState(126);
    match(ArduinoCParser::T__5);
    setState(127);
    statement();
    setState(130);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      setState(128);
      match(ArduinoCParser::T__11);
      setState(129);
      statement();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WhileStatementContext ------------------------------------------------------------------

ArduinoCParser::WhileStatementContext::WhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::ExpressionContext* ArduinoCParser::WhileStatementContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}

ArduinoCParser::StatementContext* ArduinoCParser::WhileStatementContext::statement() {
  return getRuleContext<ArduinoCParser::StatementContext>(0);
}


size_t ArduinoCParser::WhileStatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleWhileStatement;
}


std::any ArduinoCParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::WhileStatementContext* ArduinoCParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 18, ArduinoCParser::RuleWhileStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(132);
    match(ArduinoCParser::T__12);
    setState(133);
    match(ArduinoCParser::T__4);
    setState(134);
    expression();
    setState(135);
    match(ArduinoCParser::T__5);
    setState(136);
    statement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionStatementContext ------------------------------------------------------------------

ArduinoCParser::ExpressionStatementContext::ExpressionStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::ExpressionContext* ArduinoCParser::ExpressionStatementContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}


size_t ArduinoCParser::ExpressionStatementContext::getRuleIndex() const {
  return ArduinoCParser::RuleExpressionStatement;
}


std::any ArduinoCParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ExpressionStatementContext* ArduinoCParser::expressionStatement() {
  ExpressionStatementContext *_localctx = _tracker.createInstance<ExpressionStatementContext>(_ctx, getState());
  enterRule(_localctx, 20, ArduinoCParser::RuleExpressionStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(139);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324853948448) != 0)) {
      setState(138);
      expression();
    }
    setState(141);
    match(ArduinoCParser::T__3);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

ArduinoCParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::AssignmentContext* ArduinoCParser::ExpressionContext::assignment() {
  return getRuleContext<ArduinoCParser::AssignmentContext>(0);
}

ArduinoCParser::TernaryExpressionContext* ArduinoCParser::ExpressionContext::ternaryExpression() {
  return getRuleContext<ArduinoCParser::TernaryExpressionContext>(0);
}


size_t ArduinoCParser::ExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleExpression;
}


std::any ArduinoCParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ExpressionContext* ArduinoCParser::expression() {
  ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 22, ArduinoCParser::RuleExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(145);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(143);
      assignment();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(144);
      ternaryExpression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TernaryExpressionContext ------------------------------------------------------------------

ArduinoCParser::TernaryExpressionContext::TernaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::LogicalOrExpressionContext* ArduinoCParser::TernaryExpressionContext::logicalOrExpression() {
  return getRuleContext<ArduinoCParser::LogicalOrExpressionContext>(0);
}

tree::TerminalNode* ArduinoCParser::TernaryExpressionContext::QUESTION_MARK() {
  return getToken(ArduinoCParser::QUESTION_MARK, 0);
}

std::vector<ArduinoCParser::ExpressionContext *> ArduinoCParser::TernaryExpressionContext::expression() {
  return getRuleContexts<ArduinoCParser::ExpressionContext>();
}

ArduinoCParser::ExpressionContext* ArduinoCParser::TernaryExpressionContext::expression(size_t i) {
  return getRuleContext<ArduinoCParser::ExpressionContext>(i);
}

tree::TerminalNode* ArduinoCParser::TernaryExpressionContext::COLON() {
  return getToken(ArduinoCParser::COLON, 0);
}


size_t ArduinoCParser::TernaryExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleTernaryExpression;
}


std::any ArduinoCParser::TernaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitTernaryExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::TernaryExpressionContext* ArduinoCParser::ternaryExpression() {
  TernaryExpressionContext *_localctx = _tracker.createInstance<TernaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 24, ArduinoCParser::RuleTernaryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(147);
    logicalOrExpression();
    setState(153);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArduinoCParser::QUESTION_MARK) {
      setState(148);
      match(ArduinoCParser::QUESTION_MARK);
      setState(149);
      expression();
      setState(150);
      match(ArduinoCParser::COLON);
      setState(151);
      expression();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LogicalOrExpressionContext ------------------------------------------------------------------

ArduinoCParser::LogicalOrExpressionContext::LogicalOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::LogicalAndExpressionContext *> ArduinoCParser::LogicalOrExpressionContext::logicalAndExpression() {
  return getRuleContexts<ArduinoCParser::LogicalAndExpressionContext>();
}

ArduinoCParser::LogicalAndExpressionContext* ArduinoCParser::LogicalOrExpressionContext::logicalAndExpression(size_t i) {
  return getRuleContext<ArduinoCParser::LogicalAndExpressionContext>(i);
}


size_t ArduinoCParser::LogicalOrExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleLogicalOrExpression;
}


std::any ArduinoCParser::LogicalOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::LogicalOrExpressionContext* ArduinoCParser::logicalOrExpression() {
  LogicalOrExpressionContext *_localctx = _tracker.createInstance<LogicalOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 26, ArduinoCParser::RuleLogicalOrExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(155);
    logicalAndExpression();
    setState(160);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__13) {
      setState(156);
      match(ArduinoCParser::T__13);
      setState(157);
      logicalAndExpression();
      setState(162);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LogicalAndExpressionContext ------------------------------------------------------------------

ArduinoCParser::LogicalAndExpressionContext::LogicalAndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::LogicalNotExpressionContext *> ArduinoCParser::LogicalAndExpressionContext::logicalNotExpression() {
  return getRuleContexts<ArduinoCParser::LogicalNotExpressionContext>();
}

ArduinoCParser::LogicalNotExpressionContext* ArduinoCParser::LogicalAndExpressionContext::logicalNotExpression(size_t i) {
  return getRuleContext<ArduinoCParser::LogicalNotExpressionContext>(i);
}


size_t ArduinoCParser::LogicalAndExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleLogicalAndExpression;
}


std::any ArduinoCParser::LogicalAndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::LogicalAndExpressionContext* ArduinoCParser::logicalAndExpression() {
  LogicalAndExpressionContext *_localctx = _tracker.createInstance<LogicalAndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 28, ArduinoCParser::RuleLogicalAndExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(163);
    logicalNotExpression();
    setState(168);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__14) {
      setState(164);
      match(ArduinoCParser::T__14);
      setState(165);
      logicalNotExpression();
      setState(170);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LogicalNotExpressionContext ------------------------------------------------------------------

ArduinoCParser::LogicalNotExpressionContext::LogicalNotExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::LogicalNotExpressionContext* ArduinoCParser::LogicalNotExpressionContext::logicalNotExpression() {
  return getRuleContext<ArduinoCParser::LogicalNotExpressionContext>(0);
}

ArduinoCParser::BitwiseOrExpressionContext* ArduinoCParser::LogicalNotExpressionContext::bitwiseOrExpression() {
  return getRuleContext<ArduinoCParser::BitwiseOrExpressionContext>(0);
}


size_t ArduinoCParser::LogicalNotExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleLogicalNotExpression;
}


std::any ArduinoCParser::LogicalNotExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitLogicalNotExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::LogicalNotExpressionContext* ArduinoCParser::logicalNotExpression() {
  LogicalNotExpressionContext *_localctx = _tracker.createInstance<LogicalNotExpressionContext>(_ctx, getState());
  enterRule(_localctx, 30, ArduinoCParser::RuleLogicalNotExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(176);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArduinoCParser::T__15: {
        enterOuterAlt(_localctx, 1);
        setState(171);
        match(ArduinoCParser::T__15);
        setState(172);
        logicalNotExpression();
        break;
      }

      case ArduinoCParser::T__16: {
        enterOuterAlt(_localctx, 2);
        setState(173);
        match(ArduinoCParser::T__16);
        setState(174);
        logicalNotExpression();
        break;
      }

      case ArduinoCParser::T__4:
      case ArduinoCParser::T__23:
      case ArduinoCParser::IDENTIFIER:
      case ArduinoCParser::INTEGER:
      case ArduinoCParser::STRING: {
        enterOuterAlt(_localctx, 3);
        setState(175);
        bitwiseOrExpression();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BitwiseOrExpressionContext ------------------------------------------------------------------

ArduinoCParser::BitwiseOrExpressionContext::BitwiseOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::BitwiseXorExpressionContext *> ArduinoCParser::BitwiseOrExpressionContext::bitwiseXorExpression() {
  return getRuleContexts<ArduinoCParser::BitwiseXorExpressionContext>();
}

ArduinoCParser::BitwiseXorExpressionContext* ArduinoCParser::BitwiseOrExpressionContext::bitwiseXorExpression(size_t i) {
  return getRuleContext<ArduinoCParser::BitwiseXorExpressionContext>(i);
}


size_t ArduinoCParser::BitwiseOrExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleBitwiseOrExpression;
}


std::any ArduinoCParser::BitwiseOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitBitwiseOrExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::BitwiseOrExpressionContext* ArduinoCParser::bitwiseOrExpression() {
  BitwiseOrExpressionContext *_localctx = _tracker.createInstance<BitwiseOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 32, ArduinoCParser::RuleBitwiseOrExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    bitwiseXorExpression();
    setState(183);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__17) {
      setState(179);
      match(ArduinoCParser::T__17);
      setState(180);
      bitwiseXorExpression();
      setState(185);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BitwiseXorExpressionContext ------------------------------------------------------------------

ArduinoCParser::BitwiseXorExpressionContext::BitwiseXorExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::BitwiseAndExpressionContext *> ArduinoCParser::BitwiseXorExpressionContext::bitwiseAndExpression() {
  return getRuleContexts<ArduinoCParser::BitwiseAndExpressionContext>();
}

ArduinoCParser::BitwiseAndExpressionContext* ArduinoCParser::BitwiseXorExpressionContext::bitwiseAndExpression(size_t i) {
  return getRuleContext<ArduinoCParser::BitwiseAndExpressionContext>(i);
}


size_t ArduinoCParser::BitwiseXorExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleBitwiseXorExpression;
}


std::any ArduinoCParser::BitwiseXorExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitBitwiseXorExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::BitwiseXorExpressionContext* ArduinoCParser::bitwiseXorExpression() {
  BitwiseXorExpressionContext *_localctx = _tracker.createInstance<BitwiseXorExpressionContext>(_ctx, getState());
  enterRule(_localctx, 34, ArduinoCParser::RuleBitwiseXorExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(186);
    bitwiseAndExpression();
    setState(191);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__18) {
      setState(187);
      match(ArduinoCParser::T__18);
      setState(188);
      bitwiseAndExpression();
      setState(193);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BitwiseAndExpressionContext ------------------------------------------------------------------

ArduinoCParser::BitwiseAndExpressionContext::BitwiseAndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::ConditionalExpressionContext *> ArduinoCParser::BitwiseAndExpressionContext::conditionalExpression() {
  return getRuleContexts<ArduinoCParser::ConditionalExpressionContext>();
}

ArduinoCParser::ConditionalExpressionContext* ArduinoCParser::BitwiseAndExpressionContext::conditionalExpression(size_t i) {
  return getRuleContext<ArduinoCParser::ConditionalExpressionContext>(i);
}


size_t ArduinoCParser::BitwiseAndExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleBitwiseAndExpression;
}


std::any ArduinoCParser::BitwiseAndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitBitwiseAndExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::BitwiseAndExpressionContext* ArduinoCParser::bitwiseAndExpression() {
  BitwiseAndExpressionContext *_localctx = _tracker.createInstance<BitwiseAndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 36, ArduinoCParser::RuleBitwiseAndExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(194);
    conditionalExpression();
    setState(199);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__19) {
      setState(195);
      match(ArduinoCParser::T__19);
      setState(196);
      conditionalExpression();
      setState(201);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConditionalExpressionContext ------------------------------------------------------------------

ArduinoCParser::ConditionalExpressionContext::ConditionalExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::ShiftExpressionContext *> ArduinoCParser::ConditionalExpressionContext::shiftExpression() {
  return getRuleContexts<ArduinoCParser::ShiftExpressionContext>();
}

ArduinoCParser::ShiftExpressionContext* ArduinoCParser::ConditionalExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<ArduinoCParser::ShiftExpressionContext>(i);
}

ArduinoCParser::ComparisonOperatorContext* ArduinoCParser::ConditionalExpressionContext::comparisonOperator() {
  return getRuleContext<ArduinoCParser::ComparisonOperatorContext>(0);
}


size_t ArduinoCParser::ConditionalExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleConditionalExpression;
}


std::any ArduinoCParser::ConditionalExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitConditionalExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ConditionalExpressionContext* ArduinoCParser::conditionalExpression() {
  ConditionalExpressionContext *_localctx = _tracker.createInstance<ConditionalExpressionContext>(_ctx, getState());
  enterRule(_localctx, 38, ArduinoCParser::RuleConditionalExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(202);
    shiftExpression();
    setState(206);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16911433728) != 0)) {
      setState(203);
      comparisonOperator();
      setState(204);
      shiftExpression();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShiftExpressionContext ------------------------------------------------------------------

ArduinoCParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::ArithmeticExpressionContext *> ArduinoCParser::ShiftExpressionContext::arithmeticExpression() {
  return getRuleContexts<ArduinoCParser::ArithmeticExpressionContext>();
}

ArduinoCParser::ArithmeticExpressionContext* ArduinoCParser::ShiftExpressionContext::arithmeticExpression(size_t i) {
  return getRuleContext<ArduinoCParser::ArithmeticExpressionContext>(i);
}


size_t ArduinoCParser::ShiftExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleShiftExpression;
}


std::any ArduinoCParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ShiftExpressionContext* ArduinoCParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 40, ArduinoCParser::RuleShiftExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(208);
    arithmeticExpression();
    setState(213);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__20

    || _la == ArduinoCParser::T__21) {
      setState(209);
      _la = _input->LA(1);
      if (!(_la == ArduinoCParser::T__20

      || _la == ArduinoCParser::T__21)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(210);
      arithmeticExpression();
      setState(215);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArithmeticExpressionContext ------------------------------------------------------------------

ArduinoCParser::ArithmeticExpressionContext::ArithmeticExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::MultiplicativeExpressionContext *> ArduinoCParser::ArithmeticExpressionContext::multiplicativeExpression() {
  return getRuleContexts<ArduinoCParser::MultiplicativeExpressionContext>();
}

ArduinoCParser::MultiplicativeExpressionContext* ArduinoCParser::ArithmeticExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<ArduinoCParser::MultiplicativeExpressionContext>(i);
}


size_t ArduinoCParser::ArithmeticExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleArithmeticExpression;
}


std::any ArduinoCParser::ArithmeticExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitArithmeticExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ArithmeticExpressionContext* ArduinoCParser::arithmeticExpression() {
  ArithmeticExpressionContext *_localctx = _tracker.createInstance<ArithmeticExpressionContext>(_ctx, getState());
  enterRule(_localctx, 42, ArduinoCParser::RuleArithmeticExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(216);
    multiplicativeExpression();
    setState(221);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__22

    || _la == ArduinoCParser::T__23) {
      setState(217);
      _la = _input->LA(1);
      if (!(_la == ArduinoCParser::T__22

      || _la == ArduinoCParser::T__23)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(218);
      multiplicativeExpression();
      setState(223);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MultiplicativeExpressionContext ------------------------------------------------------------------

ArduinoCParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::PrimaryExpressionContext *> ArduinoCParser::MultiplicativeExpressionContext::primaryExpression() {
  return getRuleContexts<ArduinoCParser::PrimaryExpressionContext>();
}

ArduinoCParser::PrimaryExpressionContext* ArduinoCParser::MultiplicativeExpressionContext::primaryExpression(size_t i) {
  return getRuleContext<ArduinoCParser::PrimaryExpressionContext>(i);
}


size_t ArduinoCParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RuleMultiplicativeExpression;
}


std::any ArduinoCParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::MultiplicativeExpressionContext* ArduinoCParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 44, ArduinoCParser::RuleMultiplicativeExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(224);
    primaryExpression();
    setState(229);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 234881024) != 0)) {
      setState(225);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 234881024) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(226);
      primaryExpression();
      setState(231);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpressionContext ------------------------------------------------------------------

ArduinoCParser::PrimaryExpressionContext::PrimaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArduinoCParser::FunctionCallContext* ArduinoCParser::PrimaryExpressionContext::functionCall() {
  return getRuleContext<ArduinoCParser::FunctionCallContext>(0);
}

tree::TerminalNode* ArduinoCParser::PrimaryExpressionContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}

ArduinoCParser::ExpressionContext* ArduinoCParser::PrimaryExpressionContext::expression() {
  return getRuleContext<ArduinoCParser::ExpressionContext>(0);
}

tree::TerminalNode* ArduinoCParser::PrimaryExpressionContext::INTEGER() {
  return getToken(ArduinoCParser::INTEGER, 0);
}

tree::TerminalNode* ArduinoCParser::PrimaryExpressionContext::STRING() {
  return getToken(ArduinoCParser::STRING, 0);
}


size_t ArduinoCParser::PrimaryExpressionContext::getRuleIndex() const {
  return ArduinoCParser::RulePrimaryExpression;
}


std::any ArduinoCParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::PrimaryExpressionContext* ArduinoCParser::primaryExpression() {
  PrimaryExpressionContext *_localctx = _tracker.createInstance<PrimaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 46, ArduinoCParser::RulePrimaryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(248);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(232);
      functionCall();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(233);
      match(ArduinoCParser::IDENTIFIER);
      setState(238);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == ArduinoCParser::T__0) {
        setState(234);
        match(ArduinoCParser::T__0);
        setState(235);
        expression();
        setState(236);
        match(ArduinoCParser::T__1);
      }
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(240);
      match(ArduinoCParser::INTEGER);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(241);
      match(ArduinoCParser::T__23);
      setState(242);
      match(ArduinoCParser::INTEGER);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(243);
      match(ArduinoCParser::STRING);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(244);
      match(ArduinoCParser::T__4);
      setState(245);
      expression();
      setState(246);
      match(ArduinoCParser::T__5);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ComparisonOperatorContext ------------------------------------------------------------------

ArduinoCParser::ComparisonOperatorContext::ComparisonOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t ArduinoCParser::ComparisonOperatorContext::getRuleIndex() const {
  return ArduinoCParser::RuleComparisonOperator;
}


std::any ArduinoCParser::ComparisonOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitComparisonOperator(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ComparisonOperatorContext* ArduinoCParser::comparisonOperator() {
  ComparisonOperatorContext *_localctx = _tracker.createInstance<ComparisonOperatorContext>(_ctx, getState());
  enterRule(_localctx, 48, ArduinoCParser::RuleComparisonOperator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(250);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16911433728) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentContext ------------------------------------------------------------------

ArduinoCParser::AssignmentContext::AssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArduinoCParser::AssignmentContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}

std::vector<ArduinoCParser::ExpressionContext *> ArduinoCParser::AssignmentContext::expression() {
  return getRuleContexts<ArduinoCParser::ExpressionContext>();
}

ArduinoCParser::ExpressionContext* ArduinoCParser::AssignmentContext::expression(size_t i) {
  return getRuleContext<ArduinoCParser::ExpressionContext>(i);
}


size_t ArduinoCParser::AssignmentContext::getRuleIndex() const {
  return ArduinoCParser::RuleAssignment;
}


std::any ArduinoCParser::AssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitAssignment(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::AssignmentContext* ArduinoCParser::assignment() {
  AssignmentContext *_localctx = _tracker.createInstance<AssignmentContext>(_ctx, getState());
  enterRule(_localctx, 50, ArduinoCParser::RuleAssignment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(292);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(252);
      match(ArduinoCParser::IDENTIFIER);
      setState(253);
      match(ArduinoCParser::T__2);
      setState(254);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(255);
      match(ArduinoCParser::IDENTIFIER);
      setState(256);
      match(ArduinoCParser::T__0);
      setState(257);
      expression();
      setState(258);
      match(ArduinoCParser::T__1);
      setState(259);
      match(ArduinoCParser::T__2);
      setState(260);
      expression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(262);
      match(ArduinoCParser::IDENTIFIER);
      setState(263);
      match(ArduinoCParser::T__33);
      setState(264);
      expression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(265);
      match(ArduinoCParser::IDENTIFIER);
      setState(266);
      match(ArduinoCParser::T__34);
      setState(267);
      expression();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(268);
      match(ArduinoCParser::IDENTIFIER);
      setState(269);
      match(ArduinoCParser::T__35);
      setState(270);
      expression();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(271);
      match(ArduinoCParser::IDENTIFIER);
      setState(272);
      match(ArduinoCParser::T__36);
      setState(273);
      expression();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(274);
      match(ArduinoCParser::IDENTIFIER);
      setState(275);
      match(ArduinoCParser::T__37);
      setState(276);
      expression();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(277);
      match(ArduinoCParser::IDENTIFIER);
      setState(278);
      match(ArduinoCParser::T__38);
      setState(279);
      expression();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(280);
      match(ArduinoCParser::IDENTIFIER);
      setState(281);
      match(ArduinoCParser::T__39);
      setState(282);
      expression();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(283);
      match(ArduinoCParser::IDENTIFIER);
      setState(284);
      match(ArduinoCParser::T__40);
      setState(285);
      expression();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(286);
      match(ArduinoCParser::IDENTIFIER);
      setState(287);
      match(ArduinoCParser::T__41);
      setState(288);
      expression();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(289);
      match(ArduinoCParser::IDENTIFIER);
      setState(290);
      match(ArduinoCParser::T__42);
      setState(291);
      expression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionCallContext ------------------------------------------------------------------

ArduinoCParser::FunctionCallContext::FunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArduinoCParser::FunctionCallContext::IDENTIFIER() {
  return getToken(ArduinoCParser::IDENTIFIER, 0);
}

ArduinoCParser::ArgumentListContext* ArduinoCParser::FunctionCallContext::argumentList() {
  return getRuleContext<ArduinoCParser::ArgumentListContext>(0);
}


size_t ArduinoCParser::FunctionCallContext::getRuleIndex() const {
  return ArduinoCParser::RuleFunctionCall;
}


std::any ArduinoCParser::FunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::FunctionCallContext* ArduinoCParser::functionCall() {
  FunctionCallContext *_localctx = _tracker.createInstance<FunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 52, ArduinoCParser::RuleFunctionCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(294);
    match(ArduinoCParser::IDENTIFIER);
    setState(295);
    match(ArduinoCParser::T__4);
    setState(297);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324853948448) != 0)) {
      setState(296);
      argumentList();
    }
    setState(299);
    match(ArduinoCParser::T__5);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArgumentListContext ------------------------------------------------------------------

ArduinoCParser::ArgumentListContext::ArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArduinoCParser::ExpressionContext *> ArduinoCParser::ArgumentListContext::expression() {
  return getRuleContexts<ArduinoCParser::ExpressionContext>();
}

ArduinoCParser::ExpressionContext* ArduinoCParser::ArgumentListContext::expression(size_t i) {
  return getRuleContext<ArduinoCParser::ExpressionContext>(i);
}


size_t ArduinoCParser::ArgumentListContext::getRuleIndex() const {
  return ArduinoCParser::RuleArgumentList;
}


std::any ArduinoCParser::ArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitArgumentList(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::ArgumentListContext* ArduinoCParser::argumentList() {
  ArgumentListContext *_localctx = _tracker.createInstance<ArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 54, ArduinoCParser::RuleArgumentList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(301);
    expression();
    setState(306);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArduinoCParser::T__6) {
      setState(302);
      match(ArduinoCParser::T__6);
      setState(303);
      expression();
      setState(308);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

ArduinoCParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t ArduinoCParser::TypeContext::getRuleIndex() const {
  return ArduinoCParser::RuleType;
}


std::any ArduinoCParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArduinoCVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}

ArduinoCParser::TypeContext* ArduinoCParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 56, ArduinoCParser::RuleType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(309);
    _la = _input->LA(1);
    if (!(_la == ArduinoCParser::T__43

    || _la == ArduinoCParser::T__44)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void ArduinoCParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  arduinocParserInitialize();
#else
  ::antlr4::internal::call_once(arduinocParserOnceFlag, arduinocParserInitialize);
#endif
}
