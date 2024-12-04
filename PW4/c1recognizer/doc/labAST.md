## 项目结构分析

### 对项目编译流程的理解：

在这个项目中，`CMakeLists.txt`文件定义了项目的编译流程如下：

1. 进行基本设置：包括CMake版本的最低要求，编译选项，目录属性，CMake模块路径，C++标准，启用编译命令的导出功能。

2. 设置ATNLR工具的路径并查找了ANTLR包。

3. 生成词法分析器`C1Lexer`和`C1Parser`。

4. 包含生成的文件和头文件：将生成的词法分析器和语法分析器的输出目录添加到包含路径中，将项目的头文件目录、ANTLR运行时头文件目录和RapidJSON头文件目录也添加到包含路径中。
5. 创建库和可执行文件：创建了库目标`c1recognizer`并链接ANTLR运行时库，然后创建依赖于`c1recognizer`库的可执行文件，其源文件为`test/main.cpp`。
6. 完成安装：将`c1recognizer`库的可执行文件、共享库和静态库分别安装到`bin`和`lib`目录中，并将头文件安装到`include`目录中。根据模板文件生成`c1recognizer-config.cmake`配置文件，并将其安装到`cmake`目录中。



### 对项目中的文件组织及结构的理解：

在`README.md`中对项目结构的介绍的基础上修改，如下：

```
| c1recognizer	# 实验主目录
	| cmake/	# 存放CMake相关的模块和脚本
	| grammar/	# 存放文法文件，如C1Lexer.g4,C1Parser.g4,这些文件定义了C1语言的词法和语法规则
	| include/	# 存放头文件
		| antlr4-runtime/    # 存放ANTLR的运行时所需的头文件
		| c1recognizer/     # 存放C1分析器所需要的头文件
		| rapidjson/       # 存放rapidjson的运行时所需的头文件
	| src/	# 存放C1分析器的源文件
		| test/main.cpp 	# C1分析器的main源文件
		| test/lexer.cpp    # C1词法分析器的main源文件
		| test/test_cases/	# 存放具体的测试用例
	| doc/	# 存放文档，回答实验中的问题，描述实验中遇到的问题、所做的分析和设计等
	| c1r_ref_static		# 参考的C1分析器可执行程序(ubuntu)
	| Libs_for_c1r_ref/		# 参考的可执行程序运行所需的动态库
	| run_lexer.sh			# 运行C1词法分析器的bash脚本
	| CMakeLists.txt		# 编译C1分析器的cmake脚本
	| README.md				# 实验环境的部署和测试运行的命令说明。
	| 其他已有的文件
```

文件的主要用途：

- **CMakeLists.txt**：定义项目的构建流程，包括设置编译选项、查找依赖、生成词法和语法分析器、创建库和可执行文件、以及安装步骤。
- **grammar/C1Lexer.g4** 和 **grammar/C1Parser.g4**：定义C1语言的词法和语法规则，ANTLR工具根据这些文件生成相应的C++代码。
- **include/**：包含项目所需的头文件，分为ANTLR运行时头文件、C1分析器头文件和RapidJSON头文件。
- **src/**：包含C1分析器的实现文件，处理错误监听、错误报告、语法树构建和识别器逻辑。
- **test/**：包含测试相关的文件和目录，用于验证C1分析器的功能和正确性。
- **doc/**：包含实验文档，记录实验中的问题、分析和设计。
- **run_lexer.sh**：用于运行C1词法分析器的脚本，方便测试和调试。
- **c1r_ref_static** 和 **Libs_for_c1r_ref/**：提供参考的C1分析器可执行程序及其运行所需的动态库。
- **README.md**：提供项目的总体说明，包括实验环境的配置和测试运行的命令。



### 项目依赖的外部组件及用途

- **ANTLR v4**：用于生成C1语言的词法分析器和语法分析器。通过定义词法和语法规则文件（`C1Lexer.g4`和`C1Parser.g4`），ANTLR工具可以自动生成相应的C++代码，这些代码用于解析C1语言的源代码。
- **ANTLR 运行时库**：提供运行时支持，使得ANTLR生成的词法分析器和语法分析器能够正常工作。运行时库包含了必要的类和函数，用于处理解析过程中的各种操作。
- **RapidJSON**：用于处理JSON格式的数据可以用于解析配置文件、生成测试数据或处理其他需要JSON格式的数据。CmakeLists中使用命令`include_directories(${rapidjson_include_dirs})`将RapidJSON的头文件目录添加到了搜索路径中。



### 主要功能模块及模块间的交互关系：

项目主要由词法分析器，语法分析器，抽象语法树生成器，错误处理模块，运行时库，测试模块等功能模块组成。

模块间的交互关系如下：

1. **词法分析器与语法分析器**：词法分析器将输入的C1语言源代码转换为词法单元，语法分析器使用这些词法单元生成语法分析树。
2. **语法分析器与抽象语法树生成器**：语法分析器生成语法分析树，抽象语法树生成器访问该树并生成抽象语法树。
3. **错误处理模块与词法分析器、语法分析器**：错误处理模块捕获并报告词法分析器和语法分析器在分析过程中遇到的错误。
4. **运行时库与词法分析器、语法分析器**：运行时库提供词法分析器和语法分析器在运行时所需的支持功能。
5. **测试模块与词法分析器、语法分析器**：测试模块调用词法分析器和语法分析器，验证其输出是否符合预期，并报告测试结果。



## 编译过程中的问题与解决方案

编译时参照文档中的构建与测试的方法较为顺利地完成，遇到的问题有不知道`/path/to/your/antlr.jar`。使用命令`find / -name "antlr-4.13.1-complete.jar" 2>/dev/null`查询得路径为`/usr/local/lib/antlr-4.13.1-complete.jar`。



## 总结接口与C1Parser.g4文法之间的关系

在`C1Parser.g4`文法中，每个规则对应一个`Context`类，例如`exp`规则对应`C1Parser::ExpContext`。这些`Context`类提供了访问子节点的方法，例如`ctx->exp()`返回子`exp`节点的列表，`ctx->number()`返回子`number`节点。

通过Visitor Pattern，可以遍历这些`Context`对象，并根据文法规则构建相应的AST节点。例如，在`visitExp`方法中，根据`exp`规则的不同形式（如一元运算、二元运算、数字、左值等），创建不同类型的AST节点。



## 实现与调试过程中遇到的问题与解决方案

1. `visitNumber`对浮点数的处理问题：刚开始我使用`stof`来直接将`getText()`获得的内容转换为浮点数，但是存在精度问题，后来使用`std::istringstream`将字符串流中的内容提取为浮点数解决了精度问题。
2. `Segmentation fault`问题：刚开始对部分用例进行测试时出现`segmentation fault`，后来发现是因为`visitExp`中未处理$exp \rightarrow lval$的情况，添加了对左值的处理后解决了问题。



## **AST-Q1 理解访问者**

### `antlrcpp::Any` 的定义位置及其意义

`antlrcpp::Any` 定义在 `any.h` 文件中。它是一个通用类型容器，类似于 C++ 标准库中的 `std::any`，用于存储任意类型的值，并在运行时进行类型安全的访问。在 ANTLR4 中，`antlrcpp::Any` 常用于访问者模式中，以便在遍历解析树时返回不同类型的值。

### `C1Parser::ExpContext` 的含义

`C1Parser::ExpContext` 是 ANTLR4 自动生成的解析树上下文类，是`Parse Tree`中代表`exp`非终结符的节点。`ExpContext`对应于表达式规则。它包含了与该规则匹配的所有子节点的信息。

### `result`含义

`result` 是一个指向表达式语法节点的指针，根据不同的表达式类型，它可以指向不同的派生类对象，如 `binop_expr_syntax` 或 `unaryop_expr_syntax`。这些类表示具体的语法节点类型，包含了表达式的具体信息，如操作符、操作数等。

### 它们与文法的对应关系

- `C1Parser::ExpContext` 对应于文法中的表达式规则。
- `result` 对应于解析树中的具体表达式节点。

### 使用的编程接口及其含义

- `ctx->exp()`：获取该节点的子结构中所有代表`exp`非终结符的节点。
- `expressions().size()`：返回`expressions`中元素的个数，即节点包含的代表`exp`的子节点的个数。
- `ctx->getStart()`： 获取当前上下文的起始符号。
- `ctx->Plus()`， `ctx->Minus()`，`ctx->Multiply()`，`ctx->Divide()`，`ctx->Modulo()`，`ctx->LeftParen()`，`ctx->number()`：返回的是相应的终结符节点（加减乘除取模等运算符号，左括号，数字），若不存在，返回的是`nullptr`。
- `visit()`：`visit`函数在 ANTLR4 中用于遍历解析树。根据节点的类型调用相应的 `visit`方法，可以实现对解析树的递归遍历和处理。
- `as(visit(expressions[0\]))`从 `antlrcpp::Any` ：对象中提取 `expr_syntax *`类型的值。

## `result`及`as<Type>(result)`的含义

`result`是通过调用 `visit(ctx)`方法得到的 `antlrcpp::Any` 类型的对象，包含具体语法树节点。

`as(result)`是一个模板函数，用于从 `antlrcpp::Any` 对象中提取具体类型的值。`as(result)` 会将 `result`中存储的值转换为 `Type`类型的指针，并返回该指针。

## `Visitor`的实际运作机制

在本实验中，Visitor 模式用于遍历解析树并构建语法树，通过实现不同的 `visitXXX`方法，可以相应地处理不同类型的解析树节点并生成相应的语法树节点。

`syntax_tree.h`中定义了各种类型的解析树节点和相应的`visitXXX()`方法，在`syntax_tree_builder.cpp`中实现了这些方法，根据解析树节点的信息构建相应的语法树节点。在`syntax_tree.cpp`中定义了解析树节点的`accept()`方法，方法接受访问者`visitor`并将自身的引用传给访问者，访问者使用`visit()`方法访问，`visit`方法会根据当前节点的类型调用相应的 `visitXXX`方法。