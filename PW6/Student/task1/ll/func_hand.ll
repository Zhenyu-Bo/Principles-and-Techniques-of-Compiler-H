; ModuleID = 'func_test.sy'
source_filename = "func_test.sy"
; target 开始
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; target triple = "x86_64-pc-linux-gnu"
; target 结束

; add函数定义
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @add(i32 %0, i32 %1) #0 {
    ; 分配内存
    %3 = alloca i32, align 4
    %4 = alloca i32, align 4
    ; 存入参数
    store i32 %0, i32* %3, align 4
    store i32 %1, i32* %4, align 4
    ; 计算
    %5 = load i32, i32* %3, align 4
    %6 = load i32, i32* %4, align 4
    %7 = add nsw i32 %5, %6
    %8 = sub nsw i32 %7, 1
    ; 返回
    ret i32 %8
}

; main函数定义
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
    ; 分配内存
    %1 = alloca i32, align 4
    %2 = alloca i32, align 4
    %3 = alloca i32, align 4
    ; 赋值
    store i32 3, i32* %1, align 4
    store i32 2, i32* %2, align 4
    store i32 5, i32* %3, align 4
    ; 计算
    %4 = load i32, i32* %1, align 4
    %5 = load i32, i32* %2, align 4
    %6 = load i32, i32* %3, align 4
    %7 = call i32 @add(i32 %4, i32 %5) ; 调用add函数
    %8 = add nsw i32 %6, %7
    ; 返回
    ret i32 %8
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}