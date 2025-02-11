# SysYF IR

- [SysYF IR](#sysyf-ir)
  - [IR](#ir)
    - [IR Features](#ir-features)
    - [IR Format](#ir-format)
    - [Instruction](#instruction)
      - [Terminator Instructions](#terminator-instructions)
        - [Ret](#ret)
        - [Br](#br)
      - [Standard binary operators](#standard-binary-operators)
        - [Add FAdd](#add-fadd)
        - [Sub FSub](#sub-fsub)
        - [Mul FMul](#mul-fmul)
        - [SDiv FDiv](#sdiv-fdiv)
        - [SRem](#srem)
      - [Memory operators](#memory-operators)
        - [Alloca](#alloca)
        - [Load](#load)
        - [Store](#store)
      - [CastInst](#castinst)
        - [ZExt](#zext)
        - [FpToSi](#fptosi)
        - [SiToFp](#sitofp)
      - [Other operators](#other-operators)
        - [ICmp FCmp](#icmp-fcmp)
        - [Call](#call)
        - [GetElementPtr](#getelementptr)
  - [C++ APIs](#c-apis)
    - [核心类概念图](#核心类概念图)
    - [BasicBlock](#basicblock)
    - [Constant](#constant)
    - [Function](#function)
    - [GlobalVariable](#globalvariable)
    - [IRStmtBuilder](#irstmtbuilder)
    - [Instruction](#instruction-1)
    - [Module](#module)
    - [Type](#type)
    - [User](#user)
    - [Value](#value)
    - [总结](#总结)

## IR

### IR Features
- 采用类型化三地址代码的方式 
  - 区别于 X86 汇编的目标和源寄存器共用的模式： ADD EAX, EBX 
  - %2 = add i32 %0, %1
- 静态单赋值 (SSA) 形式 + 无限寄存器
  - 每个变量都只被赋值一次 
  - 容易确定操作间的依赖关系，便于优化分析
- 强类型系统
  - 每个 Value 都具备自身的类型， 
  - IR类型系统：
    - `i1`：1位宽的整数类型
    - `i32`：32位宽的整数类型
    - `float`：单精度浮点数类型
    - `pointer`：指针类型
      - 例如：`i32*, [10 x i32*]`
    - `label` bb的标识符类型
    - `functiontype`函数类型，包括函数返回值类型与参数类型（下述文档未提及）

### IR Format
下面以`easy.c`与`easy.ll`为例进行说明。  
通过命令`clang -S -emit-llvm easy.c`可以得到对应的`easy.ll`如下（其中增加了额外的注释）。`.ll`文件中注释以`;`开头。  

- `easy.c`: 
  ``` c
  int main(){
    int a;
    int b;
    a = 1;
    b = 2;
    if(a < b)
      b = 3;
    return a + b;
  }
  ```
- `easy.ll`:
  ``` c
  ; 注释: .ll文件中注释以';'开头
  ; ModuleID = 'easy.c'                                
  source_filename = "easy.c"  
  ; 注释: target的开始
  target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
  target triple = "x86_64-unknown-linux-gnu"
  ; 注释: target的结束
  
  ; 注释: 全局main函数的定义
  ; Function Attrs: noinline nounwind optnone uwtable
  define dso_local i32 @main() #0 {
  ; 注释: 第一个基本块的开始
    %1 = alloca i32, align 4
    %2 = alloca i32, align 4
    %3 = alloca i32, align 4
    store i32 0, i32* %1, align 4
    store i32 1, i32* %2, align 4
    store i32 2, i32* %3, align 4
    %4 = load i32, i32* %2, align 4
    %5 = load i32, i32* %3, align 4
    %6 = icmp slt i32 %4, %5
    br i1 %6, label %7, label %8
  ; 注释: 第一个基本块的结束

  ; 注释: 第二个基本块的开始
  7:                                                ; preds = %0
    store i32 3, i32* %3, align 4
    br label %8
  ; 注释: 第二个基本块的结束
  
  ; 注释: 第三个基本块的开始
  8:                                                ; preds = %7, %0
    %9 = load i32, i32* %2, align 4
    %10 = load i32, i32* %3, align 4
    %11 = add nsw i32 %9, %10
    ret i32 %11                                     ; 注释: 返回语句
  ; 注释: 第三个基本块的结束
  }
  
  attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
  
  !llvm.module.flags = !{!0}
  !llvm.ident = !{!1}
  
  !0 = !{i32 1, !"wchar_size", i32 4}
  !1 = !{!"clang version 10.0.1 "}
  ```
每个program由1个或多个module组成，每个module对应1个程序文件，module之间由LLVM Linker进行链接形成1个可执行文件或者库。  
每个module组成如下：
- Target Information：
  ``` c
  target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
  target triple = "x86_64-unknown-linux-gnu"
  ```
- Global Symbols: main函数的定义
- Others:尾部其他信息  

每个函数的组成如下：
- 头部：函数返回值类型、函数名、函数参数
- 一个或多个基本块：
  - 每个基本块由Label和Instruction组成。
    ``` c
    8:                                                ; preds = %7, %0
      %9 = load i32, i32* %2, align 4
      %10 = load i32, i32* %3, align 4
      %11 = add nsw i32 %9, %10
      ret i32 %11  
    ```
    这个例子中，`8`就是Label。  
    `%9 = load i32, i32* %2, align 4`中的`%9`是目的操作数，`load`是指令助记符，`i32`是`int32`的类型，`i32*`是指向`int32`的地址类型，`%2`是源操作数，`align 4`表示对齐。
### Instruction
#### Terminator Instructions
**注**：ret与br都是Terminator Instructions也就是终止指令，在llvm基本块的定义里，基本块是单进单出的，因此只能有一条终止指令（ret或br）。当一个基本块有两条终止指令，clang 在做解析时会认为第一个终结指令是此基本块的结束，并会开启一个新的匿名的基本块（并占用了下一个编号）。
##### Ret 
- 格式
  - `ret <type> <value>`
  - `ret void`
- 例子：
  - `ret i32 %0`
  - `ret void`
- 概念：` ret`指令用于将控制流（以及可选的值）从函数返回给调用者。`ret`指令有两种形式：一种返回值，然后终结函数，另一种仅终结函数。
  
##### Br 
- 格式：
  - `br i1 <cond>, label <iftrue>, label <iffalse>`
  - `br label <dest>`
- 例子：
  - `br i1 %cond label %truebb label %falsebb`
  - `br label %bb`
- 概念：`br`指令用于使控制流转移到当前功能中的另一个基本块。 该指令有两种形式，分别对应于条件分支和无条件分支。
    
#### Standard binary operators
##### Add FAdd
- 格式：
  - `<result> = add <type> <op1>, <op2>`
  - `<result> = fadd <type> <op1>, <op2>`
- 例子：
  - `%2 = add i32 %1, %0` 
  - `%2 = fadd float %1, %0` 
- 概念：`add`指令返回其两个`i32`类型的操作数之和，返回值为`i32`类型，`fadd`指令返回其两个`float`类型的操作数之和，返回值为`float`类型
  
##### Sub FSub
- 格式与例子与`add`，`fadd`类似
- 概念：`sub`指令返回其两个`i32`类型的操作数之差，返回值为`i32`类型，`fsub`指令返回其两个`float`类型的操作数之差，返回值为`float`类型

##### Mul FMul
- 格式与例子与`add`，`fadd`类似
- 概念：`mul`指令返回其两个`i32`类型的操作数之积，返回值为`i32`类型，`fmul`指令返回其两个`float`类型的操作数之积，返回值为`float`类型

##### SDiv FDiv
- 格式与例子与`add`，`fadd`类似
- 概念：`sdiv`指令返回其两个`i32`类型的操作数之商，返回值为`i32`类型，`fdiv`指令返回其两个`float`类型的操作数之商，返回值为`float`类型

##### SRem
- 格式与例子与`add`类似
- 概念：`srem`指令返回其两个`i32`类型的操作数之模，返回值为`i32`类型

#### Memory operators
##### Alloca
- 格式：`<result> = alloca <type>`
- 例子：
  - `%ptr = alloca i32`	
  - `%ptr = alloca [10 x i32]`
- 概念： `alloca`指令在当前执行函数的栈帧上分配内存，当该函数返回其调用者时将自动释放该内存。 始终在地址空间中为数据布局中指示的分配资源分配对象

##### Load
- 格式：`<result> = load <type>, <type>* <pointer>`
- 例子：`%val = load i32, i32* %ptr`
- 概念：`load`指令用于从内存中读取。

##### Store
- 格式：`store <type> <value>, <type>* <pointer>`
- 例子：`store i32 3, i32* %ptr`
- 概念：`store`指令用于写入内存

#### CastInst
##### ZExt    
- 格式：`<result> = zext <type> <value> to <type2>`
- 例子：`%1 = zext i1 %0 to i32`
- 概念：`zext`指令将其操作数**零**扩展为`type2`类型。

##### FpToSi
- 概念：`fptosi`指令将浮点值转换为`type2`（整数）类型。
- 格式：`<result> = fptosi <type> <value> to <type2>`
- 例子：`%Y = fptosi float 1.0E-247 to i32`

##### SiToFp
- 格式：`<result> = sitofp <type> <value> to <type2>`
- 例子：`%X = sitofp i32 257 to float`
- 概念：`sitofp`指令将有符号整数转换为`type2`（浮点数）类型。

#### Other operators
##### ICmp FCmp
- 格式：
  - `<result> = icmp <cond> <type> <op1>, <op2>`
    - `<cond> = eq | ne | sgt | sge | slt | sle`
  - `<result> = fcmp <cond> <type> <op1>, <op2>`
    - `<cond> = eq | ne | ugt | uge | ult | ule`
- 例子：`i1 %2 = icmp sge i32 %0, %1`
- 概念：`icmp`指令根据两个整数的比较返回布尔值，`fcmp`指令根据两个浮点数的比较返回布尔值。

##### Call
- 格式：
  - `<result> = call <return ty> <func name>(<function args>) `
- 例子：
  - `%0 = call i32 @func( i32 %1, i32* %0)`
  - `call @func( i32 %arg)`
- 概念：`call`指令用于使控制流转移到指定的函数，其传入参数绑定到指定的值。 在被调用函数中执行`ret`指令后，控制流程将在函数调用后继续执行该指令，并且该函数的返回值绑定到`result`参数。

##### GetElementPtr
- 格式：`<result> = getelementptr <type>, <type>* <ptrval> [, <type> <idx>]`
- 例子：
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 
  - `%2 = getelementptr i32, i32* %1 i32 %0` 
- 参数解释：第一个参数是计算基础类型，第二第三个参数表示索引开始的指针类型及指针，`[]`表示可重复参数，里面表示的数组索引的偏移类型及偏移值。（思考指针类型为`[10 x i32]`指针和`i32`指针`getelementptr`用法的不同）
- 概念：`getelementptr`指令用于获取数组结构的元素的地址。 它仅执行地址计算，并且不访问内存。

## C++ APIs

### 核心类概念图

![](figs/核心类概念图.png)

### BasicBlock
- 继承：从[Value](#value)继承

- 含义：基本块，是一个是单入单出的代码块，该类维护了一个指令链表，基本块本身属于 Value, 类型是 \<label\>，会被分支指令调用

- 成员：
  
  - instr_list_：指令链表
  - pre_bbs_： bb前驱集合
  - succ_bbs_：bb后继集合
  
- API: 

  ```c++
  // 创建并返回BB块，参数分别是BB块所属的Module，name是其名字默认为空，BB块所属的Function
  static Ptr<BasicBlock> create(Ptr<Module> m, const std::string &name ,Ptr<Function> parent )
  // 返回BB块所属的函数
  Ptr<Function> get_parent();
  // 返回BB块所属的Module
  Ptr<Module> get_module();
  // 返回BB块的终止指令(ret|br)，若BB块最后一条指令不是终止指令返回null
  Ptr<Instruction> get_terminator();
  // 将instr指令添加到此BB块指令链表结尾，调用IRBuilder里来创建函数会自动调用此方法
  void add_instruction(Ptr<Instruction> instr);
  // 将instr指令添加到此BB块指令链表开头
  void add_instr_begin(Ptr<Instruction> instr);
  // 将instr指令从BB块指令链表中移除，同时调用api维护好instr的操作数的use链表
  void delete_instr(Ptr<Instruction> instr);
  // BB块中指令数为空返回true
  bool empty();
  // 返回BB块中指令的数目
  int get_num_of_instr();
  //返回BB块的指令链表
  PtrList<Instruction> &get_instructions();
  // 将此BB块从所属函数的bb链表中移除
  void erase_from_parent();
      
  /****************api about cfg****************/
  std::vector<std::weak_ptr<BasicBlock>> &get_pre_basic_blocks() // 返回前驱块集合
  std::vector<std::weak_ptr<BasicBlock>> &get_succ_basic_blocks() // 返回后继块集合
  void add_pre_basic_block(Ptr<BasicBlock> bb) // 添加前驱块
  void add_succ_basic_block(Ptr<BasicBlock> bb) // 添加后继块
  void remove_pre_basic_block(Ptr<BasicBlock> bb) // 移除前驱块
  void remove_succ_basic_block(Ptr<BasicBlock> bb) // 移除后继块
  /****************api about cfg****************/
  
  ```
  
  
### Constant
- 继承：从[User](#user)继承
- 含义：常数，各种类型常量的基类
- 子类：
  - ConstantInt
    - 含义：int类型的常数
    
    - 成员
      
      - value_：常数值
      
    - API
    
      ```cpp
      int get_value() // 返回该常数类型中存的常数值
      static int get_value(Ptr<ConstantInt> const_val)// 返回该常数类型const_val中存的常数值
      static Ptr<ConstantInt> get(int val, Ptr<Module> m) // 以val值来创建常数类
      static Ptr<ConstantInt> get(bool val, Ptr<Module> m) // 以val值来创建bool常数类
      ```
    
  - ConstantFloat
    - 含义：float类型的常数
    
    - 成员
      
      - value_：常数值
      
    - API
    
      ```cpp
      static Ptr<ConstantFloat> get(float val, Ptr<Module> m) // 以val值创建并返回浮点数常量类
      float get_value() // 返回该常数类型中存的常数值
      ```
    
  - ConstantZero
    
    - 含义：用于全局变量初始化的常量0值。
    
    - API
    
      ```cpp
      static Ptr<ConstantZero> get(Ptr<Type> ty, Ptr<Module> m);// 创建并返回ConstantZero常量类
      ```
    
  - ConstantArray

    - 含义：数组类型的常数
    - 成员
        - const_array：数组常量值

    - 本次实验不需要支持常量局部数组
### Function
- 继承：从[Value](#value)继承

- 含义：函数，该类描述 LLVM 的一个简单过程，维护基本块表，格式化参数表

- 成员
  - basic_blocks_：基本块列表
  - arguments_：形参列表
  - parent_：函数属于的module
  
- API

  ```cpp
  static Ptr<Function> create(Ptr<FunctionType> ty, const std::string &name, Ptr<Module> parent);
  // 创建并返回Function，参数依次是待创建函数类型ty、函数名字name(不可为空)、函数所属的Module
  Ptr<FunctionType> get_function_type() const;
  // 返回此函数类的函数类型
  Ptr<Type> get_return_type() const;
  // 返回此函数类型的返回值类型
  void add_basic_block(Ptr<BasicBlock> bb);
  // 将bb添加至Function的bb链表上（调用bb里的创建函数时会自动调用此函数挂在function的bb链表上）
  unsigned get_num_of_args() const;
  // 得到函数形参数数量
  unsigned get_num_basic_blocks() const;
  // 得到函数基本块数量
  Ptr<Module> get_parent() const;
  // 得到函数所属的Module
  PtrList<Argument>::iterator arg_begin() 
  // 得到函数形参的list的起始迭代器
  PtrList<Argument>::iterator arg_end()
  // 得到函数形参的list的终止迭代器
  void remove(Ptr<BasicBlock>  bb)
  // 从函数的bb链表中删除一个bb
  PtrList<BasicBlock> &get_basic_blocks()
  // 返回函数bb链表
  PtrList<Argument> &get_args()
  // 返回函数的形参链表
  void set_instr_name();
  // 给函数中未命名的基本块和指令命名
  ```

  

- 相关类
  - Argument
    - 含义：参数
    
    - 成员
      - arg_no_：参数序号
      - parent_：参数属于哪个函数
      
    - API
    
      ```cpp
      Ptr<Function> get_parent() // 返回参数的所属函数
      unsigned get_arg_no() const // 返回参数在所在函数的第几个参数
      ```
### GlobalVariable
- 继承：从[User](#user)继承
- 含义：全局变量，该类用于表示全局变量，是 GlobalValue 的子类，根据地址来访问
- 成员：
  - is_const：是否为常量
  - init_val_：初始值
- API：由于SysYF语义要求所有的全局变量都默认初始化为0，故`GlobalVariable`中成员和API再构造SysYFBuilder用不到
### IRStmtBuilder
- 含义：生成IR的辅助类，该类提供了独立的接口创建各种 IR 指令，并将它们插入基本块中, 该辅助类不做任何类型检查。

- API

  ```cpp
  Ptr<BasicBlock> get_insert_block()// 返回正在插入指令的BB
  void set_insert_point(Ptr<BasicBlock> bb)// 设置当前需要插入指令的bb
  ptr<XXXInst> create_[instr_type]()// 创建instr_type(具体名字参考IRStmtBuilder.h代码)的指令并对应插入到正在插入的BB块，这种类型的指令看函数名字和参数名字和IR文档是一一对应的。
  ```

  
### Instruction
- 继承：从[User](#user)继承
- 含义：指令，该类是所有 LLVM 指令的基类，主要维护指令的操作码（指令类别），指令所属的基本块，指令的操作数个数信息
- 成员
  - parent_：指令所属的BasicBlock
  - op_id_：指令的类型id
  - num_ops_指令的操作数个数
- 子类
  - BinaryInst：双目运算指令包括add、sub、mul、div
  - 其他子类和前述文档中提到的指令一一对应，不在此赘述。
- API：所有指令的创建都要通过 IRStmtBuilder 进行，不需要关注Instruction类的实现细节，（**注**：不通过 IRStmtBuilder 来创建指令，而直接调用指令子类的创建方法未经助教完善的测试）
### Module
- 含义：一个编译单元，在此源语言的意义下是一个文件

- 成员
  - function_list_：函数链表，记录了这个编译单元的所有函数
  - global_list_：全局变量链表
  - instr_id2string_：通过指令类型id得到其打印的string
  - module_name_, source_file_name：未使用
  - 从module中能取到的基本类型
  
- API

  ```cpp
  Ptr<Type> get_void_type();
  // 得到IR中的void类型其他类型可以用类似的API得到(推荐取得类型采用lab3助教提供的方法Type::get())
  void add_function(Ptr<Function> f);
  // 将f挂在module的function链表上，在function被创建的时候会自动调用此方法来添加function
  void add_global_variable(Ptr<GlobalVariable> g);
  // 将g挂在module的GlobalVariable链表上，在GlobalVariable被创建的时候会自动调用此方法来添加GlobalVariable
  PtrList<GlobalVariable> &get_global_variable();
  // 获取全局变量列表
  std::string get_instr_op_name( Instruction::OpID instr )；
  // 获取instr对应的指令名(打印ir时调用)
  void set_print_name();
  // 设置打印ir的指令与bb名字；
  ```
  
  
### Type
- 含义：IR的类型，该类是所有类型的超类

- 成员
  
  - tid_：枚举类型，表示type的类型（包含VoidType、LabelType、FloatType、Int1、Int32、ArrayType、PointerType）
  
- 子类
  - IntegerType
    - 含义：int 类型
    
    - 成员
      
      - num_bits：长度（i1或者i32）
      
    - API
    
      ```cpp
      unsigned get_num_bits();// 返回int的位数
      ```
  - FloatType
    
    - 含义：float 类型
  - FunctionType
    - 含义：函数类型
    
    - 成员
      - result_：返回值类型
      - args_：参数类型列表
      
    - API
    
      ```cpp
      static Ptr<FunctionType> get(Ptr<Type> result, PtrVec<Type> params);
      // 返回函数类型，参数依次是返回值类型result，形参类型列表params
      unsigned get_num_of_args() const;
      // 返回形参个数
      Ptr<Type> get_param_type(unsigned i) const;
      // 返回第i个形参的类型
      Ptr<Type> get_return_type() const;
      // 返回函数类型中的返回值类型
      ```
  - ArrayType
    - 含义：数组类型
    
    - 成员
      - contained_：数组成员的类型
      - num_elements_：数组维数
      
    - API
    
      ```cpp
      static Ptr<ArrayType> get(Ptr<Type> contained, unsigned num_elements);
      // 返回数组类型，参数依次是 数组元素的类型contained，数组元素个数num_elements
      Ptr<Type> get_element_type() const
      // 返回数组元素类型
      unsigned get_num_of_elements() const
      // 返回数组元素个数
      ```
  - PointerType
    - 含义：指针类型
    
    - 成员
      
      - contained_：指针指向的类型
      
    - API
    
      ```cpp
      Ptr<Type> get_element_type() const { return contained_; }
      // 返回指针指向的类型
      static Ptr<PointerType> get(Ptr<Type> contained);
      // 返回contained类型的指针类型
      Ptr<Type> get_pointer_element_type();// 若是PointerType则返回指向的类型，若不是则返回nullptr。
      static Ptr<PointerType> create(Ptr<Type> contained);
      // 创建指向contained类型的指针类型
      ```
  
- API

  ```cpp
  bool is_void_type()// 判断是否是void类型其他类型有类似API请查看Type.h
  static Ptr<Type> get_void_type(Ptr<Module> m);// 得到void类型
  Ptr<Type> get_pointer_element_type();// 若是PointerType则返回指向的类型，若不是则返回nullptr。
  Ptr<Type> get_array_element_type();// 若是ArrayType则返回指向的类型，若不是则返回nullptr。
  ```
  
  
### User
- 继承：从[value](#value)继承

- 含义：使用者，提供一个操作数表，表中每个操作数都直接指向一个 Value, 提供了 use-def 信息，它本身是 Value 的子类， Value 类会维护一个该数据使用者的列表，提供def-use信息。简单来说操作数表表示我用了谁，该数据使用者列表表示谁用了我。这两个表在后续的**优化实验**会比较重要请务必理解。

- 成员
  - operands_：参数列表，表示这个使用者所用到的参数
  - num_ops_：表示该使用者使用的参数的个数
  
- API

  ```cpp
  Ptr<Value> get_operand(unsigned i) const;
  // 从user的操作数链表中取出第i个操作数
  void set_operand(unsigned i, Ptr<Value> v);
  // 将user的第i个操作数设为v
  void add_operand(Ptr<Value> v);
  // 将v挂到User的操作数链表上
  unsigned get_num_operand() const;
  // 得到操作数链表的大小
  void remove_use_of_ops();
  // 从User的操作数链表中的所有操作数处的use_list_ 移除该User;
  void remove_operands(int index1, int index2);
  // 移除操作数链表中索引为index1-index2的操作数，例如想删除第0个操作数：remove_operands(0,0)
  ```

### Value 
- 含义：最基础的类，代表一个操作数，代表一个可能用于指令操作数的带类型数据

- 成员
  - use_list_：记录了所有使用该操作数的指令的列表
  - name_：名字
  - type_：类型，一个type类，表示操作数的类型
  
- API

  ```cpp
  Ptr<Type> get_type() const //返回这个操作数的类型
  std::list<Use> &get_use_list() // 返回value的使用者链表
  void add_use(Ptr<Value> val, unsigned arg_no = 0);
  // 添加val至this的使用者链表上
  void replace_all_use_with(Ptr<Value> new_val);
  // 将this在所有的地方用new_val替代，并且维护好use_def与def_use链表
  void remove_use(Ptr<Value> val);
  // 将val从this的use_list_中移除
  template <typename T>
  Ptr<T> as();
  // Ptr<Value> value通过value->as<Function>()转为子类型指针Ptr<Function>，封装了dynamic_pointer_cast
  ```

### 总结

在本文档里提供了为SysYF语言程序生成LLVM IR可能需要用到的SysYF IR应用编程接口，如果对这些API有问题的请移步issue讨论，本次`SysYF IR`应用编程接口由助教自行设计实现，并做了大量测试，如有对助教的实现方法有异议或者建议的也请移步issue讨论。
