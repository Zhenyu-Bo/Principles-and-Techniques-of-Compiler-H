grammar regular;

regex: expression EOF ;

expression: choice;

choice: concat ('|' concat)*;

concat: closure+;

closure: atom '*' | atom;

atom: '(' expression ')' | LETTER;

LETTER: [a-z];

WS: [ \t\r\n]+ -> skip;
