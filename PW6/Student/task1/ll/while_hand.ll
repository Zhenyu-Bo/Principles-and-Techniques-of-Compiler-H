; ModuleID = 'while_test.c'
source_filename = "while_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; 为全局变量分配内存
@a = dso_local global i32 0, align 4
@b = dso_local global i32 0, align 4

; 全局main函数定义
define dso_local i32 @main() #0 {
    ; 初始化
    store i32 0, i32* @b, align 4
    store i32 3, i32* @a, align 4
    br label %1

1:                                                      ; preds = %0, %4
    ; 加载a的值
    %2 = load i32, i32* @a, align 4
    ; 判断a是否大于0
    %3 = icmp sgt i32 %2, 0
    ; 如果a大于0，跳转到5
    br i1 %3, label %4, label %9

4:                                                      ; preds = %1
    ; 加载a, b的值
    %5 = load i32, i32* @a, align 4
    %6 = load i32, i32* @b, align 4
    ; 计算并存储
    %7 = add nsw i32 %6, %5
    store i32 %7, i32* @b, align 4
    %8 = sub nsw i32 %5, 1
    store i32 %8, i32* @a, align 4
    br label %1

9:                                                      ; preds = %1
    ; 返回b的值
    %10 = load i32, i32* @b, align 4
    ret i32 %10
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}