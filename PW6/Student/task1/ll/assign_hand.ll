; ModuleID = 'assign_test.sy'
source_filename = "assign_test.sy"
; target 开始
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; target triple = "x86_64-pc-linux-gnu"
; target 结束

; 全局main函数的定义
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
    %1 = alloca float, align 4 ; 为b分配内存
    %2 = alloca [2 x i32], align 4 ; 为数组a分配内存
    ; 初始化
    store float 0x3FFCCCCCC0000000, float* %1, align 4 ; b = 1.8
    %3 = getelementptr inbounds [2 x i32], [2 x i32]* %2, i32 0, i32 0 ; 获取数组a的第一个元素的地址
    store i32 2, i32* %3, align 4 ; a[0] = 2
    ; 计算
    %4 = load i32, i32* %3, align 4 ; 获取a[0]
    %5 = sitofp i32 %4 to float ; 转换为float
    %6 = load float, float* %1, align 4 ; 获取b
    %7 = fmul float %5, %6 ; a[0] * b
    %8 = fptosi float %7 to i32 ; 转换为int
    ; 结果存入a[1]
    %9 = getelementptr inbounds [2 x i32], [2 x i32]* %2, i32 0, i32 1 ; 获取数组a的第二个元素的地址
    store i32 %8, i32* %9, align 4 ; a[1] = a[0] * b
    ; 返回a[1]
    %10 = load i32, i32* %9, align 4 ; 获取a[1]
    ret i32 %10
}

; Function Attrs: argmemonly nofree nounwind willreturn
attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}