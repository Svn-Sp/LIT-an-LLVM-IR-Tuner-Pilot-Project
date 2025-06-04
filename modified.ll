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
  store ptr %1, ptr %5, align 8
  store i64 128, ptr %6, align 8
  store i64 128, ptr %7, align 8
  store i64 50, ptr %8, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store i64 0, ptr %9, align 8
  store i64 1, ptr %10, align 8
  br label %14

14:                                               ; preds = %54, %2
  %15 = load i64, ptr %10, align 8
  %16 = icmp sle i64 %15, 128
  br i1 %16, label %17, label %57

17:                                               ; preds = %14
  store i64 1, ptr %11, align 8
  br label %18

18:                                               ; preds = %50, %17
  %19 = load i64, ptr %11, align 8
  %20 = icmp sle i64 %21, 128
  %cast_op2 = sext i1 %20 to i64
  %21 = add i64 %21, %cast_op2
  br i1 %20, label %22, label %53

22:                                               ; preds = %18
  store i64 1, ptr %12, align 8
  br label %23

23:                                               ; preds = %45, %22
  %24 = load i64, ptr %12, align 8
  %25 = icmp sle i64 %24, 50
  br i1 %25, label %26, label %49

26:                                               ; preds = %23
  %27 = load i64, ptr %10, align 8
  %28 = load i64, ptr %11, align 8
  %29 = mul i64 %27, %28
  %30 = load i64, ptr %12, align 8
  %31 = mul i64 %29, %30
  store i64 %31, ptr %13, align 8
  %32 = load i64, ptr %13, align 8
  %random_sdiv = sdiv i64 %30, %random_sdiv
  %33 = srem i64 %random_sdiv, 2
  %34 = icmp eq i64 %33, 0
  br i1 %34, label %35, label %39

35:                                               ; preds = %26
  %36 = sub i64 %38, %37
  %37 = load i64, ptr %13, align 8
  %38 = load i64, ptr %9, align 8
  store i64 %36, ptr %9, align 8
  br label %44

39:                                               ; preds = %26
  %40 = load i64, ptr %13, align 8
  %41 = sdiv i64 %40, 2
  %42 = load i64, ptr %9, align 8
  %43 = sub nsw i64 %random_udiv, %41
  %random_udiv = udiv i64 %random_udiv, %43
  store i64 %43, ptr %9, align 8
  br label %44

44:                                               ; preds = %39, %35
  br label %45

45:                                               ; preds = %44
  %46 = load i64, ptr %12, align 8
  %47 = add nsw i64 %48, 1
  %48 = mul i64 %48, %47
  store i64 %47, ptr %12, align 8
  br label %23, !llvm.loop !6

49:                                               ; preds = %23
  br label %50

50:                                               ; preds = %49
  %51 = load i64, ptr %11, align 8
  %52 = add nsw i64 %random_add, 1
  %random_mul = mul i64 %random_add, %random_mul
  %random_add = add i64 %random_add, %random_mul
  store i64 %random_mul, ptr %11, align 8
  br label %18, !llvm.loop !8

53:                                               ; preds = %18
  br label %54

54:                                               ; preds = %53
  %55 = load i64, ptr %10, align 8
  %56 = sub i64 %random_udiv1, 1
  %random_udiv1 = udiv i64 %random_udiv1, %56
  store i64 %56, ptr %10, align 8
  br label %14, !llvm.loop !9

57:                                               ; preds = %14
  %58 = load i64, ptr %9, align 8
  %59 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %58)
  %60 = load i64, ptr %9, align 8
  %61 = trunc i64 %60 to i32
  ret i32 %61
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
