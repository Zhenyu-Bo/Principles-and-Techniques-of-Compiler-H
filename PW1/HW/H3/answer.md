对`clang -S -Og -m64 sort.c -o sort.s`生成的汇编代码`sort.s`的排序函数部分注释如下：

```asm
bubbleSort:                             # @bubbleSort
    .cfi_startproc                      # 标记函数开始
# %bb.0:
    cmpl	$2, %esi                    # n存储在esi中，比较 esi 和 2，如果 esi < 2，则无需排序，跳转到.LBB0_8直接返回
    jl	.LBB0_8
# %bb.1:
    addl	$-1, %esi                   # esi = esi - 1，将esi赋值为n - 1
    xorl	%r8d, %r8d                  # r8d = 0，初始化 r8d 为 0，r8d用于存储循环变量i（r8d表示r8的低32位）
    movl	%esi, %r9d                  # r9d = esi，初始化为n - 1，r9d用于存储n - 1 - i即第二层循环的循环边界（r9d是r9的低32位）
    jmp	.LBB0_2                      	# 跳转到 .LBB0_2
    .p2align	4, 0x90                 # 对齐指令
.LBB0_7:                                #   in Loop: Header=BB0_2 Depth=1
    addl	$1, %r8d                    # r8d = r8d + 1
    addl	$-1, %r9d                   # r9d = r9d - 1
    cmpl	%esi, %r8d                  # 比较 r8d 和 esi，如果 r8d == esi即i = n - 1，外层循环结束，跳转到.LBB0_8返回
    je	.LBB0_8
.LBB0_2:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_4 Depth 2
    movl	%r9d, %r9d                  # r9d = r9d，冗余指令
    cmpl	%r8d, %esi                  # 比较 r8d 和 esi，如果 r8d <= esi，跳转到 .LBB0_7
    jle	.LBB0_7
# %bb.3:                                #   in Loop: Header=BB0_2 Depth=1
    xorl	%edx, %edx                  # edx = 0，初始化 edx 为 0
    jmp	.LBB0_4                      	# 跳转到 .LBB0_4
    .p2align	4, 0x90                 # 对齐指令
.LBB0_6:                                #   in Loop: Header=BB0_4 Depth=2
    movq	%rax, %rdx                  # rdx = rax，复制 rax 到 rdx，实现rax存储的值为rdx+1
    cmpq	%rax, %r9                   # 比较 rax 和 r9，如果 rax == r9，跳转到 .LBB0_7
    je	.LBB0_7
.LBB0_4:                                #   Parent Loop BB0_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
    movl	(%rdi,%rdx,4), %r10d        # r10d = *(rdi + rdx * 4)，加载数组元素到 r10d，rdi存储传入的数组基址，r10d存储a[j]，rdx存储循环变量j
    movl	4(%rdi,%rdx,4), %ecx        # ecx = *(rdi + rdx * 4 + 4)，加载数组下一个元素到 ecx，ecx存储a[j + 1]
    leaq	1(%rdx), %rax               # rax = rdx + 1，计算下一个索引即j+1
    cmpl	%ecx, %r10d                 # 比较 r10d 和 ecx，如果 r10d <= ecx，则不需要做任何处理，跳转到.LBB0_6
    jle	.LBB0_6
# %bb.5:                                #   in Loop: Header=BB0_4 Depth=2
    movl	%ecx, (%rdi,%rdx,4)         # *(rdi + rdx * 4) = ecx即a[j] = ecx，交换数组元素
    movl	%r10d, 4(%rdi,%rdx,4)       # *(rdi + rdx * 4 + 4) = r10d即a[j + 1] = r10d，交换数组元素
    jmp	.LBB0_6                      	# 跳转到 .LBB0_6
.LBB0_8:
    retq                                # 返回
.Lfunc_end0:
    .size	bubbleSort, .Lfunc_end0-bubbleSort # 定义 bubbleSort 函数的大小
    .cfi_endproc                       	# 标记函数结束
                                        # -- End function
    .globl	main                        # -- Begin function main
    .p2align	4, 0x90                 # 对齐指令
    .type	main,@function
```

其中传入的数组基址和数组元素个数分别存储在`rdi`和`esi`中，循环变量`i`，`j`分别存储在`r8d(r8的低32位)`和`rdx`中，`rax`存储`j+1`的值，内层循环结束后执行`rdx = rax`以实现`j++`，`r9d(r9的低32位)`用于存储`n-i-1`，`ecx`用于数组元素`a[j]`，`r10d`用于存储`a[j+1]`，交换两个数值不需要再引入中间变量`temp`，因为可以直接将寄存器中的值存储到相应的地址中。
