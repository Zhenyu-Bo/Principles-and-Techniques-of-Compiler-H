lexer grammar C1Lexer;

tokens {
    Comma,
    SemiColon,
    Assign,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,
    If,
    Else,
    While,
    Const,
    Equal,
    NonEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,

    Int,
    Float,
    Void,

    Identifier,
    FloatConst,
    IntConst
}

Comma: ',';
SemiColon: ';';
Assign: '=';
LeftBracket: '[';
RightBracket: ']';
LeftBrace: '{';
RightBrace: '}';
LeftParen: '(';
RightParen: ')';
If: 'if';
Else: 'else';
While: 'while';
Const: 'const';
Equal: '==';
NonEqual: '!=';
Less: '<';
Greater: '>';
LessEqual: '<=';
GreaterEqual: '>=';
Plus: '+';
Minus: '-';
Multiply: '*';
Divide: '/';
Modulo: '%';

Int: 'int';
Float: 'float';
Void: 'void';

Identifier: [_a-zA-Z] [_0-9a-zA-Z]*;

IntConst: '0' [0-7]*
        | [1-9] [0-9]*
        | '0' [xX] [0-9a-fA-F]+;

FloatConst: ([0-9]* '.' [0-9]+ | [0-9]+ '.') ([eE] [-+]? [0-9]+)?
          | [0-9]+ [eE] [-+]? [0-9]+;

LineComment: ('//' | '/\\' ('\r'? '\n') '/') ~[\r\n\\]* ('\\' ('\r'? '\n')? ~[\r\n\\]*)* '\r'? '\n' -> skip;
BlockComment: '/*' .*? '*/' -> skip;

WhiteSpace: [ \t\r\n]+ -> skip;
