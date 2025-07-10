grammar ArduinoC;

// Parser Rules
program
    : (declaration | functionDefinition | functionDeclaration)* EOF
    ;

declaration
    : type IDENTIFIER ('[' INTEGER ']')? ('=' expression)? ';'
    ;

functionDefinition
    : type IDENTIFIER '(' parameterList? ')' compoundStatement
    ;

functionDeclaration
    : type IDENTIFIER '(' parameterList? ')' ';'
    ;

parameterList
    : parameter (',' parameter)*
    ;

parameter
    : type IDENTIFIER
    ;

compoundStatement
    : '{' statement* '}'
    ;

statement
    : expressionStatement
    | compoundStatement
    | declaration
    | ifStatement
    | whileStatement
    | returnStatement
    ;

returnStatement
    : 'return' expression? ';'
    ;

ifStatement
    : 'if' '(' expression ')' statement ('else' statement)?
    ;

whileStatement
    : 'while' '(' expression ')' statement
    ;

expressionStatement
    : expression? ';'
    ;

expression
    : assignment
    | ternaryExpression
    ;

ternaryExpression
    : logicalOrExpression ('?' expression ':' expression)?
    ;

logicalOrExpression
    : logicalAndExpression ('||' logicalAndExpression)*
    ;

logicalAndExpression
    : logicalNotExpression ('&&' logicalNotExpression)*
    ;

logicalNotExpression
    : '!' logicalNotExpression
    | '~' logicalNotExpression
    | bitwiseOrExpression
    ;

bitwiseOrExpression
    : bitwiseXorExpression ('|' bitwiseXorExpression)*
    ;

bitwiseXorExpression
    : bitwiseAndExpression ('^' bitwiseAndExpression)*
    ;

bitwiseAndExpression
    : conditionalExpression ('&' conditionalExpression)*
    ;

conditionalExpression
    : shiftExpression (comparisonOperator shiftExpression)?
    ;

shiftExpression
    : arithmeticExpression (('<<' | '>>') arithmeticExpression)*
    ;

// Proper arithmetic expression hierarchy with left-recursion for operator chaining
arithmeticExpression
    : multiplicativeExpression (('+'|'-') multiplicativeExpression)*
    ;

multiplicativeExpression 
    : primaryExpression (('*'|'/'|'%') primaryExpression)*
    ;

primaryExpression
    : functionCall
    | IDENTIFIER ('[' expression ']')?
    | INTEGER
    | '-' INTEGER  // Support for negative numbers
    | STRING
    | '(' expression ')'
    ;

// Removed arithmeticOperator rule - using direct tokens in expressions for clarity

comparisonOperator
    : '=='
    | '!='
    | '<'
    | '>'
    | '<='
    | '>='
    ;

assignment
    : IDENTIFIER '=' expression
    | IDENTIFIER '[' expression ']' '=' expression
    | IDENTIFIER '+=' expression
    | IDENTIFIER '-=' expression
    | IDENTIFIER '*=' expression
    | IDENTIFIER '/=' expression
    | IDENTIFIER '%=' expression
    | IDENTIFIER '&=' expression
    | IDENTIFIER '|=' expression
    | IDENTIFIER '^=' expression
    | IDENTIFIER '<<=' expression
    | IDENTIFIER '>>=' expression
    ;

functionCall
    : IDENTIFIER '(' argumentList? ')'
    ;

argumentList
    : expression (',' expression)*
    ;

type
    : 'int'
    | 'void'
    ;

// Lexer Rules
QUESTION_MARK : '?';
COLON : ':';

IDENTIFIER
    : [a-zA-Z_][a-zA-Z0-9_]*
    ;

INTEGER
    : '0x' [0-9a-fA-F]+  // Hexadecimal literals
    | [0-9]+
    ;

STRING
    : '"' (~["\r\n])* '"'
    ;

// Whitespace and Comments
WS
    : [ \t\r\n]+ -> skip
    ;

COMMENT
    : '//' ~[\r\n]* -> skip
    ;

BLOCK_COMMENT
    : '/*' .*? '*/' -> skip
    ;