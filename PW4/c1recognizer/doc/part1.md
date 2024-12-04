[TOC]

---
####任务描述

本关任务：阅读理解 C1 语言的基本描述并回答问题。编写一些 C1 程序（请先自行保存，在后面关卡中再提交），并用现有的 C 编译器进行编译和运行。

####基本知识
C1 语言是本实训要实现的实验编程语言，它是 C 语言的一个子集。它没有完善的类型系统，只有`int`、`float`和元素为`int`或`float`类型的一维数组，并附带`const`描述符。它的函数定义没有参数和返回值。

##### C1 语言的文法

C1 语言的文法采用`EBNF (Extended Backus-Naur Form)`表示如下：

```
CompUnit    → [ CompUnit ] ( Decl | FuncDef ) 
Decl        → ConstDecl 
            | VarDecl
ConstDecl   → 'const' BType ConstDef { ',' ConstDef } ';'
BType       → 'int' | 'float'
ConstDef    → Ident '=' Exp
            | Ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
VarDecl     → BType VarDef { ',' VarDef } ';'
VarDef      → Ident 
            | Ident '[' Exp ']' 
            | Ident '=' Exp
            | Ident '[' [ Exp ] ']' '=' '{' Exp { ',' Exp } '}'
FuncDef     → void Ident '(' ')' Block
Block       → '{' { BlockItem } '}'
BlockItem   → Decl 
            | Stmt
Stmt        → LVal '=' Exp ';'
            | Ident '(' ')' ';'
            | Block
            | 'if' '( Cond ')' Stmt [ 'else' Stmt ]
            | 'while' '(' Cond ')' Stmt
            | ';'
LVal        → Ident
            | Ident '[' Exp ']'
Cond        → Exp RelOp Exp
RelOp       → '==' | '!=' | '<' | '>' | '<=' | '>='
Exp         → Exp BinOp Exp
            | UnaryOp Exp
            | '(' Exp ')'
            | LVal
            | Number
Number      → FloatConst
            | IntConst
BinOp       → '+' | '−' | '*' | '/' | '%'
UnaryOp     → '+' | '−'
```

**EBNF 中的符号含义**

* `[...]`：内包含的为可选项；

* `{...}`：内包含的为可重复 0 次至无数次的项。

##### C1 语言的特征

除上述文法之外，C1 语言还具有如下特征：

* C1 语言中运算符的优先级及结合性与 C 语言一致；

* C1 语言中标识符`Ident`规范为 C 语言标识符规范子集，请参考 [`ISO/IEC 9899`](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1124.pdf) 第 51 页关于标识符的定义。并将该规范中非终结符`identifier`产生式修改为 `identifier-nondigit: nondigit`，这里不考虑`universal-cha\fracter-name`、`other implementation-defined cha\fracters`等；

* C1 语言中同名标识符的约定：
  - 全局变量和局部变量的作用域可以重叠，重叠部分局部变量优先，同名局部变量的作用域不能重叠；

  - C1 语言中变量名可以与函数名相同。

* C1 语言中注释的规范与 C 语言一致，请参考[`ISO/IEC 9899`](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1124.pdf) 第 66 页关于注释的定义，其中：

```c
// ...     # 这是一个单行注释

# 这实际是一个两行的注释
/* ...
*/
```

* C1 语言中数值常量可以是整型数`IntConst`，也可以是浮点数`FloatConst`。
  - 整型数参考[`ISO/IEC 9899`](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1124.pdf) 第 54 页关于整型常量的定义，在此基础上忽略所有后缀；

  - 浮点数参考[`ISO/IEC 9899`](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1124.pdf)第 57 页关于浮点常量的定义，在此基础上上忽略 16 进制浮点表示和所有后缀。