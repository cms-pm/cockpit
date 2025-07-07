grammar ArduinoC;

// Parser Rules
program
    : (declaration | functionDefinition)* EOF
    ;

declaration
    : type IDENTIFIER ('=' expression)? ';'
    ;

functionDefinition
    : type IDENTIFIER '(' parameterList? ')' compoundStatement
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
    | logicalOrExpression
    | conditionalExpression
    | arithmeticExpression
    | functionCall
    | IDENTIFIER
    | INTEGER
    | STRING
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

arithmeticExpression
    : primaryExpression arithmeticOperator primaryExpression
    ;

primaryExpression
    : functionCall
    | IDENTIFIER
    | INTEGER
    | STRING
    | '(' expression ')'
    ;

arithmeticOperator
    : '+'
    | '-'
    | '*'
    | '/'
    | '%'
    ;

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
IDENTIFIER
    : [a-zA-Z_][a-zA-Z0-9_]*
    ;

INTEGER
    : [0-9]+
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