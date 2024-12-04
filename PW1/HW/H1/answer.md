1. 区别是将 a 赋值为了-4而不是4。原因是`-DNEG`选项会在预处理阶段定义`NEG`宏，根据条件编译会将宏`M`定义为-4，故而`a = M`会将 a 赋值为-4。

2. 区别及原因：
   1. 寄存器名称。32位汇编文件使用的寄存器有`ebp`, `esp`, `eax`，都以`e`开头，而64位汇编文件使用的寄存器有`rbp`, `rsp`, `eax`。其原因是64位有16个寄存器，32位只有8个。但是32位前8个都有不同的命名，分别是e_，而64位前8个使用了r代替e，也就是r_。e开头的寄存器命名依然可以直接运用于相应寄存器的低32位。而剩下的寄存器名则是从r8 - r15。
   2. 使用的指令。32位汇编文件使用了`movl`, `pushl`等指令，而64位汇编文件中相应地使用了`movq`, `pushq`等指令，其原因是数据存储的位宽不同。`movl`, `pushl`均用于处理32位数据，而64位汇编文件中需要对64位的数据进行处理，所以使用了处理64位数据的`movq`，`pushq`指令。
   3. 栈桢设置。32 位汇编中，使用`pushl %ebp` 和 `movl %esp, %ebp` 设置栈帧。64 位汇编中，使用`pushq %rbp` 和 `movq %rsp, %rbp` 设置栈帧。（但是在64位汇编中没有专门存放栈桢指针的寄存器，`rbp`是通用寄存器）原因同上。
   4. 栈指针调整。32 位汇编中，用于`subl $16, %esp` 调整栈指针。而64 位汇编中，没有类似的栈指针调整指令。这是因为64位汇编中大多数函数在调用的一开始就分配全部所需栈空间，之后保持栈指针不改变，不需要调整栈指针。
   5. CFI指令偏移量。32位汇编中，CFA相对于当前栈指针（%esp）向上偏移了8个字节，而在64位汇编中CFA相对于当前栈指针（%rsp）向上偏移了16个字节。这是位宽的不同造成的。
   6. 辅助函数与函数调用：32 位汇编中，定义了一个辅助函数 `__x86.get_pc_thunk.ax`，用于获取程序计数器并且有 `call __x86.get_pc_thunk.ax` 和 `addl $_GLOBAL_OFFSET_TABLE_, %eax`用于函数调用。而64 位汇编中，没有类似的辅助函数和函数调用指令。这是因为32 位架构中需要通过辅助函数获取全局偏移表地址，而 64 位架构中不需要。
   7. 结束指令。32 位汇编中，使用 leave 和 ret 指令。64 位汇编中，使用 popq %rbp 和 ret 指令。这是因为32位汇编需要恢复基指针和栈指针，这由`leave`指令来完成，实际上`leave`指令等价于以下两条指令的组合：`movl %ebp, %esp`, `popl %ebp`，而64位汇编未调整栈指针，所以只需要恢复基指针，这由`poop %rbp`来完成。
   8. 控制流保护。64 位汇编中，有 endbr64 指令，用于控制流保护。而32位汇编中没有。这是因为64 位架构中引入了 `endbr64` 指令，用于增强控制流保护，防止某些类型的攻击。
   9. 段声明。64 位汇编中，有额外的段声明 .section .note.gnu.property,"a"，用于存储GNU 属性。这是因为64位汇编中为了支持更大的地址空间、提高兼容性和安全性、优化性能以及提供更好的调试和维护支持而引入了额外的段声明。

3. 各个输出的异同如下：
   1. sample.i
      * 相同点：源代码部分完全相同，都是一个简单的C程序，将宏M替换成了相应的数值。
   
      * 不同点：两个文件在预处理指令部分有所不同。gcc 生成的文件包含标准C预定义头文件 stdc-predef.h 的处理信息。而clang 生成的文件包含更多的内建头文件处理信息。它们的输出如下：
   
        gcc：
   
        ```c
        # 0 "sample.c"
        # 0 "<built-in>"
        # 0 "<command-line>"
        # 1 "/usr/include/stdc-predef.h" 1 3 4
        # 0 "<command-line>" 2
        # 1 "sample.c"
        ```
   
        clang：
   
        ```c
        # 1 "sample.c"
        # 1 "<built-in>" 1
        # 1 "<built-in>" 3
        # 361 "<built-in>" 3
        # 1 "<command line>" 1
        # 1 "<built-in>" 2
        # 1 "sample.c" 2
        ```
   
   2. sample.s
   
      gcc和clang生成的汇编文件完全相同
   
   3. 反汇编
   
      对gcc和clang生成的汇编文件进行反汇编得到的文件大体上相同，在一些地方上有细微差别，比如使用的指令，立即数，指令的数目等有所不同。
   
   4. sample.o
   
      gcc和clang生成的可执行文件基本相同，在一些地方上有细微差别，由于是二进制文件，不便描述区别。
   
   