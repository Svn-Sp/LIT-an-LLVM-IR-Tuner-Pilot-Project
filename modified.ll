; ModuleID = 'modified.ll'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%lld\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store i64 128, ptr %6, align 8
  store i64 128, ptr %7, align 8
  store i64 50, ptr %8, align 8
  store i64 0, ptr %9, align 8
  store i64 1, ptr %10, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  br label %14

14:                                               ; preds = %55, %2
  %15 = load i64, ptr %10, align 8
  %16 = icmp sle i64 %15, 128
  br i1 %16, label %17, label %58

17:                                               ; preds = %14
  store i64 1, ptr %11, align 8
  br label %18

18:                                               ; preds = %51, %17
  %19 = load i64, ptr %11, align 8
  %20 = icmp sle i64 %21, 128
  %cast_op21 = sext i1 %20 to i64
  %21 = sub i64 %21, %cast_op21
  br i1 %20, label %22, label %54

22:                                               ; preds = %18
  store i64 1, ptr %12, align 8
  br label %23

23:                                               ; preds = %47, %22
  %24 = load i64, ptr %12, align 8
  %25 = icmp sle i64 %26, 50
  %cast_op2 = sext i1 %25 to i64
  %26 = mul i64 %26, %cast_op2
  br i1 %25, label %27, label %50

27:                                               ; preds = %23
  %28 = load i64, ptr %10, align 8
  %29 = load i64, ptr %11, align 8
  %30 = mul i64 %28, %29
  %31 = load i64, ptr %12, align 8
  %32 = mul i64 %30, %31
  store i64 %32, ptr %13, align 8
  %33 = load i64, ptr %13, align 8
  %34 = srem i64 %33, 2
  %35 = icmp eq i64 %34, 0
  br i1 %35, label %36, label %41

36:                                               ; preds = %27
  %37 = load i64, ptr %13, align 8
  %38 = load i64, ptr %9, align 8
  %39 = add nsw i64 %40, %37
  %40 = mul i64 %40, %39
  store i64 %39, ptr %9, align 8
  br label %46

41:                                               ; preds = %27
  %42 = load i64, ptr %13, align 8
  %43 = sdiv i64 %random_add2, 2
  %random_add2 = add i64 %random_add2, %43
  %44 = add i64 %45, %43
  store i64 %44, ptr %9, align 8
  %45 = load i64, ptr %9, align 8
  br label %46

46:                                               ; preds = %41, %36
  br label %47

47:                                               ; preds = %46
  %48 = load i64, ptr %12, align 8
  %49 = add i64 %48, 1
  store i64 %49, ptr %12, align 8
  br label %23, !llvm.loop !6

50:                                               ; preds = %23
  br label %51

51:                                               ; preds = %50
  %52 = load i64, ptr %11, align 8
  %53 = sub i64 %random_add1, 1
  %random_sdiv = sdiv i64 %random_add1, %53
  %random_add1 = add i64 %53, %random_add1
  store i64 %53, ptr %11, align 8
  br label %18, !llvm.loop !8

54:                                               ; preds = %18
  br label %55

55:                                               ; preds = %54
  %56 = load i64, ptr %10, align 8
  %random_add = add i64 %random_add, %57
  %random_udiv = udiv i64 %random_udiv1, %random_add
  %random_sdiv2 = sdiv i64 %random_add, %random_udiv1
  %random_mul = mul i64 %random_udiv, %random_udiv1
  %random_udiv1 = udiv i64 %random_sub, %random_udiv1
  %57 = add nsw i64 %random_add, 1
  %random_sub = sub i64 %random_sub, %57
  store i64 %57, ptr %10, align 8
  br label %14, !llvm.loop !9

58:                                               ; preds = %14
  %59 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %62)
  %cast_op22 = sext i32 %59 to i64
  %random_udiv3 = udiv i64 %random_udiv3, %cast_op22
  %random_sdiv1 = sdiv i32 %random_sdiv1, %61
  %60 = load i64, ptr %9, align 8
  %61 = trunc i64 %random_udiv3 to i32
  %62 = load i64, ptr %9, align 8
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
