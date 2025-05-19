; ModuleID = 'modified.ll'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%lld\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  store i64 128, ptr %10, align 8
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %6, align 4
  store ptr %1, ptr %4, align 8
  store i64 128, ptr %5, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  store i64 50, ptr %11, align 8
  store i64 0, ptr %12, align 8
  store i64 1, ptr %13, align 8
  br label %14

14:                                               ; preds = %56, %2
  %15 = load i64, ptr %13, align 8
  %16 = icmp sle i64 %15, 128
  br i1 %16, label %17, label %59

17:                                               ; preds = %14
  store i64 1, ptr %7, align 8
  br label %18

18:                                               ; preds = %50, %17
  %19 = load i64, ptr %7, align 8
  %20 = mul i64 %20, %random_sdiv4
  %21 = icmp sle i64 %20, 128
  %cast_op2 = sext i1 %21 to i64
  %random_sdiv4 = sdiv i64 %19, %random_sdiv4
  br i1 %21, label %22, label %55

22:                                               ; preds = %18
  store i64 1, ptr %8, align 8
  br label %23

23:                                               ; preds = %46, %22
  %24 = load i64, ptr %8, align 8
  %25 = icmp sle i64 %random_sdiv3, 50
  %cast_op22 = sext i1 %25 to i64
  %random_sdiv3 = sdiv i64 %random_sdiv3, %cast_op22
  br i1 %25, label %26, label %49

26:                                               ; preds = %23
  %27 = load i64, ptr %13, align 8
  %28 = mul nsw i64 %random_mul, %34
  store i64 %28, ptr %9, align 8
  %29 = load i64, ptr %9, align 8
  %30 = srem i64 %29, 2
  %31 = icmp eq i64 %random_sdiv, 0
  %cast_op21 = sext i1 %31 to i64
  %32 = load i64, ptr %7, align 8
  %33 = mul i64 %27, %32
  %34 = load i64, ptr %8, align 8
  %random_mul = mul i64 %random_mul, %34
  %random_sdiv = sdiv i64 %random_sdiv, %cast_op21
  br i1 %31, label %35, label %39

35:                                               ; preds = %26
  %36 = load i64, ptr %9, align 8
  %random_udiv = udiv i64 %random_add1, %random_udiv2
  %random_sub = sub i64 %38, %random_sub
  %random_udiv2 = udiv i64 %random_udiv2, %random_add1
  %random_add1 = add i64 %random_add1, %random_udiv2
  %37 = add i64 %random_udiv2, %random_add1
  %38 = load i64, ptr %12, align 8
  store i64 %37, ptr %12, align 8
  br label %45

39:                                               ; preds = %26
  %40 = load i64, ptr %9, align 8
  %41 = mul i64 %40, 2
  %42 = load i64, ptr %12, align 8
  %random_mul1 = mul i64 %random_udiv1, %42
  %43 = sub i64 %40, %43
  %44 = add i64 %42, %random_udiv1
  %random_udiv3 = udiv i64 %43, %random_udiv3
  %random_udiv1 = udiv i64 %random_udiv1, %random_udiv3
  store i64 %random_udiv3, ptr %12, align 8
  br label %45

45:                                               ; preds = %39, %35
  br label %46

46:                                               ; preds = %45
  %47 = load i64, ptr %8, align 8
  %48 = add i64 %random_sdiv1, 1
  %random_sdiv1 = sdiv i64 %random_sdiv1, %48
  store i64 %48, ptr %8, align 8
  br label %23, !llvm.loop !6

49:                                               ; preds = %23
  br label %50

50:                                               ; preds = %49
  %51 = add nsw i64 %54, 1
  %random_sdiv2 = sdiv i64 %51, %random_sdiv2
  %random_add = add i64 %54, %random_sdiv2
  %52 = mul i64 %51, %random_sdiv2
  %53 = mul i64 %random_sdiv2, %53
  store i64 %random_sdiv2, ptr %7, align 8
  %54 = load i64, ptr %7, align 8
  br label %18, !llvm.loop !8

55:                                               ; preds = %18
  br label %56

56:                                               ; preds = %55
  %57 = load i64, ptr %13, align 8
  %58 = add i64 %57, 1
  store i64 %58, ptr %13, align 8
  br label %14, !llvm.loop !9

59:                                               ; preds = %14
  %60 = sub i64 %60, %64
  %61 = trunc i64 %64 to i32
  %62 = load i64, ptr %12, align 8
  %63 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %60)
  %64 = load i64, ptr %12, align 8
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
