# 使用 ANTLR-4.13.1 为 C1 语言构造分析器

在本实验中，你将学习并逐步掌握如何为一个编程语言构建一个能生成语法树的分析器。你将通过编写描述语言文法的描述文件，使用 [ANTLR v4](http://www.antlr.org/) 生成相应的分析器源码，该分析器能为合法的输入程序输出对应的分析树(`parse tree`)。然后，你会继续编写为分析树生成相应抽象语法树(`abstract syntax tree, AST`)的访问者(`visitor`)，从而最终实现一个能为合法的输入程序输出 AST 的分析器。
<br>

本实训要实现的编程语言是 C1 语言，你需要先阅读 **第 1 关 预备知识：C1 语言** 来了解 C1 语言的特点，尝试编写一些 C1 程序，再利用现有的 C 语言编译器来编译。然后，你将通过逐级递进的闯关任务，逐步完成对 C1 语言分析器的构造，它将为正确的 C1 程序输出对应的语法分析树。
<br>

**版本库结构**

本实验提供的版本库的主目录是`c1recognizer`，其目录结构如下：

```
- <your repo>
  | c1recognizer     # 本实验在版本库中的主目录
    | cmake/
>>  | grammar/       # 存放文法文件，如C1Lexer.g4
    | include/
       | antlr4-runtime/    # 存放ANTLR的运行时所需的头文件
       | c1recognizer/     # 存放C1分析器所需要的头文件
       | rapidjson/       # 存放rapidjson的运行时所需的头文件
>>  | src/           # 存放C1分析器的源文件
    | test/main.cpp    # C1分析器的main源文件
    | test/lexer.cpp    # C1词法分析器的main源文件
>>  | test/test_cases/    # 增加你的测试程序
>>  | doc/     # 增加文档：回答实验中的关卡问题，描述实验中遇到的问题、所做的分析和设计等
    | c1r_ref_static     # 参考的C1分析器可执行程序(ubuntu)
    | Libs_for_c1r_ref/   # 参考的可执行程序运行所需的动态库
    | run_lexer.sh       # 运行C1词法分析器的bash脚本
    | CMakeLists.txt     # 编译C1分析器的cmake脚本
    | README.md		   #实验环境的部署，测试运行的命令（很重要！！！）
    | 其他已有的文件
```
你需要在上述标记有`>>`的目录中增加或修改文件，来完成本实训的实践任务。
<br>

在每个实践任务中，会有进一步的实践任务检查说明，助教会借助脚本实现对实践任务的半自动检查，请一定严格按照各实践任务的要求组织实训项目的目录和文件，否则会影响本实训的成绩。
>注：引用的外部库：[`rapidjson`](https://github.com/Tencent/rapidjson)、[`antlr4-runtime(cpp)`](https://github.com/antlr/antlr4/tree/master/runtime/Cpp)。

**本地实验的环境配置方法**

平台已配置好实训项目所需的各种软件，其中`antlr-4.13.1-complete.jar`在`/usr/local/lib`目录下：

**1. Java环境安装**

ANTLR 工具需要 JVM 才能执行，此外为了方便使用 ANTLR 的`grun`，你需要一个能够编译`java`源文件的环境。因此，你需要一个完整的`Java Development Kit`。

  - 如果你使用 Linux，推荐通过包管理器安装 OpenJDK 8。在你的包管理器中通过搜索来确定包名，如`Ubuntu`下包名为`openjdk-8-jdk`，安装之即可；

  - 如果你使用 Mac，你需要手动安装一个 JDK。

**2. ANTLR下载与安装**
  - 你需要从[`ANTLR Download`](https://github.com/antlr/website-antlr4/tree/gh-pages/download)下载`antlr-4.13.1-complete.jar`；

  - 你需要将该`jar`包的存放路径加入到环境变量`CLASSPATH`中，即可以在`Bash`中执行`export CLASSPATH=".:/path/to/your/antlr-4.13.1-complete.jar:$CLASSPATH"`；

  - 你可以考虑将这一命令加入`.bashrc`（对于`Bash`），以省去你每次配置的麻烦。

**3. 定义`antlr4`和`grun`工具**

  - 可以定义别名`antlr4`表示 ANTLR 工具，即`alias antlr4='java org.antlr.v4.Tool'`；

  - `grun`本质上是一个别名，可以定义如下：`alias grun='java org.antlr.v4.runtime.misc.TestRig'`或`alias grun='java org.antlr.v4.gui.TestRig'`；

  - 同样的，你可以将这些别名命令加入到`.bashrc`，以节省你配置和使用的时间。

四个关卡的文档在[doc](./doc/)目录下。
