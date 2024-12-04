parser grammar C1Parser;
options { tokenVocab = C1Lexer; }

compilationUnit: (decl | funcdef)+;
decl: constdecl | vardecl;
btype: Int | Float;
constdecl: Const btype constdef ( Comma constdef )* SemiColon;
constdef: Identifier Assign exp
        | Identifier LeftBracket ( exp )? RightBracket Assign LeftBrace exp ( Comma exp )* RightBrace;
vardecl: btype vardef ( Comma vardef )* SemiColon;
vardef: Identifier
      | Identifier LeftBracket exp RightBracket
      | Identifier Assign exp
      | Identifier LeftBracket ( exp )? RightBracket Assign LeftBrace exp ( Comma exp )* RightBrace;
funcdef: Void Identifier LeftParen RightParen block;
block: LeftBrace ( blockItem )* RightBrace;
blockItem: decl | stmt;
stmt: lval Assign exp SemiColon
    | Identifier LeftParen RightParen SemiColon
    | block
    | If LeftParen cond RightParen stmt ( Else stmt )?
    | While LeftParen cond RightParen stmt
    | SemiColon
    ;
lval: Identifier
    | Identifier LeftBracket exp RightBracket
    ;
cond: exp relop exp;
relop: Equal | NonEqual | Less | Greater | LessEqual | GreaterEqual;
exp:
    (Plus | Minus) exp
    | exp (Multiply | Divide | Modulo) exp
    | exp (Plus | Minus) exp
    | LeftParen exp RightParen
    | number
    | lval
;
number: FloatConst | IntConst;