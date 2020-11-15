; ModuleID = 'while.c'
source_filename = "while.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; while should not be a problem. Impl it like asm.
; Be careful about numbers. As for the order, take the below as an example.

; int main()
; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @main() #0 {
  ; int a;
  %1 = alloca i32
  ; int i;
  %2 = alloca i32
  ; a = 10;
  store i32 10, i32* %1
  ; i = 0;
  store i32 0, i32* %2
  ; while (i < 10)
  br label %3
3:
  %4 = load i32, i32* %2
  %5 = icmp slt i32 %4, 10
  br i1 %5, label %6, label %12
6:
  ; i = i + 1;
  %7 = load i32, i32* %2
  %8 = add i32 %7, 1
  store i32 %8, i32* %2
  ; a = a + i;
  %9 = load i32, i32* %1
  %10 = load i32, i32* %2
  %11 = add i32 %9, %10
  store i32 %11, i32* %1
  br label %3
12:
  ; return a;
  %13 = load i32, i32* %1
  ret i32 %13
}

attributes #0 = { noinline nounwind optnone sspstrong uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{!"clang version 11.0.0"}
