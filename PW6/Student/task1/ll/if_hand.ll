; ModuleID = 'if_test.sy'
source_filename = "if_test.sy"
; target 开始
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; target triple = "x86_64-pc-linux-gnu"
; target 结束

; 为全局变量a分配内存
@a = dso_local global i32 0, align 4

; 全局main函数的定义
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
    store i32 10, i32* @a, align 4 ; a = 10
    %1 = load i32, i32* @a, align 4 ; 获取a
    %2 = icmp sgt i32 %1, 0 ; 判断a是否大于0
    %3 = alloca i32, align 4 ; 为返回值分配内存
    br i1 %2, label %4, label %6

4:                                                          ; preds = %0
    %5 = load i32, i32* @a, align 4 ; 获取a
    store i32 %5, i32* %3, align 4 ; 存入返回值
    br label %7

6:                                                          ; preds = %0
    store i32 0, i32* %3, align 4 ; 存入返回值
    br label %7

7:                                                          ; preds = %6, %4
    %8 = load i32, i32* %3, align 4 ; 获取返回值
    ret i32 %8 ; 返回
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}