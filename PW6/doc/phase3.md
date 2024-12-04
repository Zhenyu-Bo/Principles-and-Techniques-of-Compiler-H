[TOC]

---

### 任务描述
**本关任务**：编写[IRBuilder.cpp](../src/SysYFIRBuilder/IRBuilder.cpp)文件，实现低级中间代码生成器，为SysYF语言程序生成兼容的LLVM IR代码。

### 相关知识

#### SysYF语言定义

SysYF语言定义见`SysYF语言定义.pdf`。该文档包含了部分语义说明(如同名标识符的约定等)。

> 同第二关，为了减小实验难度，本实验中限制数组只能是一维数组，并且声明的数组长度必须是单个字面量数字，不能是其他表达式。但是访问数组元素时下标可以使用复杂的表达式。
> 同时全局变量在初始化时必须使用字面量数字初始化（或者不显式初始化），不能是其他常量表达式。

```cpp
// 以下都是全局变量
const int a = 10;          // OK
const int b = a;           // Error
const int c = 10 + 1;      // Error
int arr[2] = {1, 2}; // OK
int brr[2] = {0, a}; // Error
```

#### 实验框架

本实训项目提供用C++语言编写的SysYF IR 应用编程库，用于构建LLVM IR的子集。为了简化你的实验，本实训的实验框架代码已完成了SysYF源程序到 C++ 上的抽象语法树的转换。

##### Scope

在[IRBuilder.h](../include/SysYFIRBuilder/IRBuilder.h)中，还定义了一个用于存储作用域的类`Scope`。它的作用是在遍历语法树时，辅助管理不同作用域中的变量。它提供了以下接口：
```cpp
// 进入一个新的作用域
void enter();
// 退出一个作用域
void exit();
// 往当前作用域插入新的名字->值映射
bool push(std::string name, Ptr<Value> val);
// 根据名字，以及是否为函数的bool值寻找到对应值
// isfunc 为 true 时为寻找函数，否则为寻找其他变量对应的值
Ptr<Value> find(std::string name, bool isfunc);
// 判断当前是否在全局作用域内
bool in_global();
```
你需要根据语义合理调用`enter`与`exit`，并且在变量声明和使用时正确调用`push`与`find`。在类`SysYFIRBuilder`中，有一个`Scope`类型的成员变量`scope`，它在初始化时已经将特殊函数加入了作用域中。因此，你在进行名字查找时不需要顾虑是否需要对特殊函数进行特殊操作。

##### shared_ptr
为了防止内存泄漏，助教将框架中的裸指针换成了智能指针，相关的类型定义在`include/internal_types`中，其中以下的类型转换方法：
```cpp
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;
```
static_pointer_cast和dynamic_pointer_cast用于智能指针的类型转换，隐式完成了拆包、类型转换、重新包装和内存控制块维护，const_pointer_cast在转换时去除const属性，含义和使用方法类似于static_cast、dynamic_cast和const_cast：
```cpp
std::vector<std::shared_ptr<Duck>> ducks;
ducks.push_back(std::static_pointer_cast<Duck>(std::make_shared<EdibleDuck>()));
```

同时为了简化代码，助教为 `SysYFIR` 中的节点类型（`BasicBlock`，`Function`，`Instruction` 等）提供了 `as` 函数，你可以使用 `as` 实现 `dynamic_pointer_cast` 的效果：

```cpp
std::shared_ptr<Instruction> inst = ...;
// 把 Instruction 转为子类 BinaryInst
// 相当于 dynamic_pointer_cast<BinaryInst>(inst)
inst->as<BinaryInst>();
```

##### shared_from_this

这是助教在更新代码框架时涉及的部分，在实验中并不会用到，有兴趣的同学了解即可。由于改成了智能指针，我们在使用create方法创建IR中的Value对象时，需要在初始化的时候生成它的智能指针并返回，我们使用了`include/internal_macros.h`中的宏来定义使用了智能指针后新的初始化过程，先创建该对象的智能指针，然后对该智能指针调用init方法，而在init的过程中可能会用到它自身的智能指针，比如[src/SysYFIR/BasicBlock.cpp](../src/SysYFIR/BasicBlock.cpp)中的`parent_->add_basic_block(dynamic_pointer_cast<BasicBlock>(shared_from_this()));`，这时需要用shared_from_this，原因参考[cpp smart pointer](https://www.cyhone.com/articles/right-way-to-use-cpp-smart-pointer/)。

当然，如果你企图直接使用构造函数而不用create方法的话，你会很惊讶地发现你做不到，因为助教已经很贴心地将构造函数设为private或者protected了，所以不用担心不小心的使用，而之所以不在构造函数中使用，原因参考[shared_from_this说明1](https://blog.csdn.net/weixin_38927079/article/details/115505724)和[shared_from_this说明2](https://blog.csdn.net/u012398613/article/details/52243764)

### 本关具体任务
1. 你需要在`src/SysYFIRBuilder`文件夹中，调用SysYF IR应用编程接口，填写[IRBuilder.cpp](../src/SysYFIRBuilder/IRBuilder.cpp)文件，以实现 LLVM IR 的自动生成。注意以下几点：
   * a. 创建include/SysYFIR中的对象时，只能通过create方法，而不能直接使用构造函数(原因参考[shared_from_this](#shared_from_this))，如下：
      ```cpp
      auto fun = Function::create(fun_type, node.name, module);
      ```
      而不是：
      ```cpp
      auto fun = new Function(fun_type, node.name, module);
      ```
   * b. 尽量不要使用裸指针，而是使用shared_ptr和相关的类型转换方法，即`include/internal_types`定义的类型和方法
2. 在`report.md`内回答<a href="#思考题">思考题</a>
3. 在`contribution.md`内由组长填写各组员的贡献比

### 任务分解与语言特征分解

SysYF语言涉及表达式、常量、数组、if、while、break、continue等特征。你可以使用`return`语句, `put_int`, `put_float`函数等为每一类特征编写测试样例，在确保每一类测例没有问题的情况下组合不同的特征进行测试。

为了分工协作，可能需要提前实现一些相对粗糙的visit函数以方便如if, while, break等特性的实现。

你可以参考[pku minic](https://pku-minic.github.io/online-doc/#/)进行分工实现。

### 编译、运行与验证

#### 编译运行 SysYFCompiler

```sh
mkdir build
cd build
cmake ..
make
```
请注意，你会发现CMakeLists.txt中的CMAKE_CXX_FLAGS多了很多参数，我们介绍其中一部分(参考[gcc warning options](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html))：

   * -Wextra：打印一些额外的warning信息
   * -Wno-unused：不警告未使用的变量，框架中一些函数有一些不用的参数
   * -Wshadow：如果一个局部变量隐藏掉了其他的变量、参数等，报告warning

编译后会产生 `SysYFCompiler` 程序，它能将SysYF源程序（sy文件）输出为LLVM IR。  
当需要对 sy 文件测试时，可以这样使用：

```sh
SysYFCompiler test.sy -emit-ir -o test.ll
```
得到对应的ll文件。

为了方便同学们 debug，在执行 `cmake ..` 时可以指定 `CMAKE_BUILD_TYPE=Asan` 来启用 [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)：

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Asan
make
```

AddressSanitizer 是一个内存错误检测器，它可以检测出如下 Bug：

* 对堆、栈和全局变量的越界访问
* 释放后使用（use-after-free）
* 多次释放 （Double Free）

#### 链接SysYF库

[lib/lib.c](../lib/lib.c)是SysYF的运行时库，包含`get_int`, `put_int`等库函数。

如果直接使用clang编译test.ll(由SysYFCompiler生成)来生成二进制会出现如下错误：

```bash
$ clang test.ll -o test
test.ll:(.text+0x20): undefined reference to `put_int'
clang-15: error: linker command failed with exit code 1 (use -v to see invocation)
```

这是因为llvm没有找到在test.ll中声明的`put_int`函数所对应的定义。为了解决这个问题，我们需要使用clang同时编译[lib/lib.c](../lib/lib.c)和test.ll并链接形成二进制。可以使用如下命令编译生成二进制：

```bash
clang test.ll lib/lib.c -o test
```

#### 自动测试

本实训项目提供了自动评测脚本, 在[Student/task3/](../Student/task3/)目录下执行`python3 test.py`, 即可得到评测信息。

你可以在[test.py](../Student/task3/test.py)的`TEST_DIRS`中添加测试目录。默认启用的测试目录为[test](../Student/task3/test/)目录。你可以将自行编写的测例放在[Student/task3/test_stu/](../Student/task3/test_stu/)中。

在测试目录中，你需要提供`<case>.sy`, `<case>.out`文件。其中`<case>.sy`文件是程序，`<case>.out`文件是程序的期望输出。必要时你还需要提供`<case>.in`文件，作为`<case>.sy`输入。

测试样例除了位于版本库中的公开测例外,还包含不开放的隐藏测例。

平台上第三关的评测只判断公开测例是否完全通过, 第三关的分数由助教线下检查后，根据公开测例和隐藏测例的通过情况确定, 因此请自行设计合法测例测试你们的代码，确保你们的代码考虑了足够多情况以通过尽可能多的隐藏测例。

### 思考题
请在[report/report.md](../report/report.md)中详细回答下述思考题。

3-1. 在`scope`内单独处理`func`的好处有哪些。


### 选做

本实训项目提供了选做内容, 若你能完成选做部分, 将会有额外加分(仅针对本次实验的团队代码得分, 并且分数不能超过该部分得分上限)。

选做部分验收方式为线下验收，你需要在线下检查时提供对应代码通过助教给出的选做部分测试样例，并且讲解你的代码。

选做部分说明如下:


| 项目  | 计科 | 网安  |
| :------------: | :------------: | :------------: |
| 将一维数组指针作为参数  | 必做  | 选做  |
|  逻辑运算(\&\&, \|\|, \!), 重点考察短路计算 | 必做  | 选做  |

#### 数组指针参数 & 逻辑运算

目前给出的SysYF IR接口支持数组指针参数和逻辑运算的短路计算，因此你不需要修改接口。  
注意`pointer`和`array`的区别以及文法中`&&`和`||`的优先级。

### 备注

本次实验不需要考虑局部常量数组。

本关提供了参考实现 `sysyf_ref`，可以参考它生成的 IR。
