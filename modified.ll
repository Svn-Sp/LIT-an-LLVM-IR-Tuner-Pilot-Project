; ModuleID = 'modified.ll'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%lld\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store i64 128, ptr %6, align 8
  store i64 128, ptr %7, align 8
  store i64 50, ptr %8, align 8
  store i64 0, ptr %9, align 8
  store i64 1, ptr %10, align 8
  br label %14

14:                                               ; preds = %52, %2
  %15 = load i64, ptr %10, align 8
  %16 = icmp sle i64 %15, 128
  br i1 %16, label %17, label %55

17:                                               ; preds = %14
  store i64 1, ptr %11, align 8
  br label %18

18:                                               ; preds = %48, %17
  %19 = load i64, ptr %11, align 8
  %20 = icmp sle i64 %19, 128
  br i1 %20, label %21, label %51

21:                                               ; preds = %18
  store i64 1, ptr %12, align 8
  br label %22

22:                                               ; preds = %44, %21
  %23 = load i64, ptr %12, align 8
  %24 = icmp sle i64 %23, 50
  br i1 %24, label %25, label %47

25:                                               ; preds = %22
  %26 = load i64, ptr %10, align 8
  %27 = load i64, ptr %11, align 8
  %28 = mul i64 %26, %27
  %29 = load i64, ptr %12, align 8
  %30 = mul nsw i64 %28, %29
  store i64 %30, ptr %13, align 8
  %31 = load i64, ptr %13, align 8
  %32 = srem i64 %31, 2
  %33 = icmp eq i64 %32, 0
  br i1 %33, label %34, label %38

34:                                               ; preds = %25
  %35 = load i64, ptr %13, align 8
  %36 = load i64, ptr %9, align 8
  %37 = sub i64 %36, %35
  store i64 %37, ptr %9, align 8
  br label %43

38:                                               ; preds = %25
  %39 = load i64, ptr %13, align 8
  %40 = sdiv i64 %39, 2
  %41 = load i64, ptr %9, align 8
  %42 = sub nsw i64 %41, %40
  store i64 %42, ptr %9, align 8
  br label %43

43:                                               ; preds = %38, %34
  br label %44

44:                                               ; preds = %43
  %45 = load i64, ptr %12, align 8
  %46 = add nsw i64 %45, 1
  store i64 %46, ptr %12, align 8
  br label %22, !llvm.loop !6

47:                                               ; preds = %22
  br label %48

48:                                               ; preds = %47
  %49 = load i64, ptr %11, align 8
  %50 = sub i64 %49, 1
  store i64 %50, ptr %11, align 8
  br label %18, !llvm.loop !8

51:                                               ; preds = %18
  br label %52

52:                                               ; preds = %51
  %53 = load i64, ptr %10, align 8
  %54 = add nsw i64 %53, 1
  store i64 %54, ptr %10, align 8
  br label %14, !llvm.loop !9

55:                                               ; preds = %14
  %56 = load i64, ptr %9, align 8
  %57 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %56)
  %58 = load i64, ptr %9, align 8
  %59 = trunc i64 %58 to i32
  ret i32 %59
}

declare i32 @printf(ptr noundef, ...) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 21.0.0git (https://github.com/llvm/llvm-project.git 8b40a09bf50d62e2017611b7be2c55fab22d9572)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
