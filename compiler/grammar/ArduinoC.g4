grammar ArduinoC;

// Parser Rules
program
    : (declaration | functionDefinition)* EOF
    ;

declaration
    : type IDENTIFIER ';'
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
    ;

expressionStatement
    : expression? ';'
    ;

expression
    : assignment
    | functionCall
    | IDENTIFIER
    | INTEGER
    | STRING
    ;

assignment
    : IDENTIFIER '=' expression
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