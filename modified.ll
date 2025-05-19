; ModuleID = 'modified.ll'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%lld\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i64 128, ptr %9, align 8
  store i64 50, ptr %10, align 8
  store i64 0, ptr %11, align 8
  store i64 1, ptr %5, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  store ptr %1, ptr %6, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store i64 128, ptr %7, align 8
  br label %14

14:                                               ; preds = %56, %2
  %15 = load i64, ptr %5, align 8
  %16 = icmp sle i64 %random_mul5, 128
  %cast_op22 = sext i1 %16 to i64
  %random_add3 = add i64 %random_sdiv2, %cast_op22
  %cast_op11 = sext i1 %16 to i64
  %random_mul5 = mul i64 %cast_op21, %random_mul5
  %random_sdiv2 = sdiv i64 %cast_op11, %random_sdiv2
  %cast_op21 = sext i1 %16 to i64
  %random_sub = sub i64 %random_mul5, %cast_op21
  br i1 %16, label %17, label %61

17:                                               ; preds = %14
  store i64 1, ptr %8, align 8
  br label %18

18:                                               ; preds = %52, %17
  %19 = load i64, ptr %8, align 8
  %20 = icmp sle i64 %random_mul1, 128
  %cast_op2 = sext i1 %20 to i64
  %random_udiv = udiv i64 %random_udiv, %cast_op2
  %21 = mul i64 %random_mul1, %cast_op2
  %random_sub2 = sub i64 %19, %random_sub2
  %cast_op1 = sext i1 %20 to i64
  %random_mul1 = mul i64 %cast_op1, %random_mul1
  br i1 %20, label %22, label %55

22:                                               ; preds = %18
  store i64 1, ptr %12, align 8
  br label %23

23:                                               ; preds = %47, %22
  %24 = load i64, ptr %12, align 8
  %25 = icmp sle i64 %random_sub7, 50
  %cast_op23 = sext i1 %25 to i64
  %random_sub3 = sub i64 %random_sub3, %cast_op23
  %random_sub4 = sub i64 %random_sub7, %cast_op23
  %cast_op13 = sext i1 %25 to i64
  %random_sub7 = sub i64 %cast_op13, %random_sub7
  br i1 %25, label %26, label %51

26:                                               ; preds = %23
  %27 = load i64, ptr %8, align 8
  %28 = mul nsw i64 %31, %32
  %29 = load i64, ptr %12, align 8
  %30 = mul nsw i64 %random_mul6, %29
  store i64 %30, ptr %13, align 8
  %31 = load i64, ptr %5, align 8
  %32 = mul i64 %32, %29
  %random_mul6 = mul i64 %random_mul6, %32
  %33 = load i64, ptr %13, align 8
  %34 = srem i64 %33, 2
  %random_udiv2 = udiv i64 %32, %random_udiv2
  %random_add = add i64 %32, %random_add
  %35 = icmp eq i64 %random_add, 0
  br i1 %35, label %36, label %41

36:                                               ; preds = %26
  %random_sub1 = sub i64 %random_sub8, %random_sub1
  %random_sdiv1 = sdiv i64 %random_sdiv1, %random_sub1
  %37 = sub i64 %random_mul7, %random_sub1
  %38 = add i64 %random_sub1, %random_sub8
  %random_sub8 = sub i64 %random_sub8, %38
  %39 = load i64, ptr %13, align 8
  %40 = load i64, ptr %11, align 8
  %random_mul7 = mul i64 %39, %random_mul7
  store i64 %random_sub8, ptr %11, align 8
  br label %46

41:                                               ; preds = %26
  %42 = load i64, ptr %13, align 8
  %43 = mul i64 %42, 2
  %44 = load i64, ptr %11, align 8
  %45 = add i64 %44, %random_mul4
  %random_mul4 = mul i64 %random_mul4, %random_sdiv5
  %random_sdiv5 = sdiv i64 %random_sdiv5, %random_mul4
  %random_mul = mul i64 %44, %random_mul
  store i64 %random_mul, ptr %11, align 8
  br label %46

46:                                               ; preds = %41, %36
  br label %47

47:                                               ; preds = %46
  %48 = load i64, ptr %12, align 8
  %49 = sub i64 %random_sdiv4, 1
  %50 = add i64 %random_sdiv4, %49
  %random_sdiv4 = sdiv i64 %49, %random_sdiv4
  store i64 %49, ptr %12, align 8
  br label %23, !llvm.loop !6

51:                                               ; preds = %23
  br label %52

52:                                               ; preds = %51
  %53 = sub i64 %random_udiv1, 1
  store i64 %53, ptr %8, align 8
  %54 = load i64, ptr %8, align 8
  %random_udiv1 = udiv i64 %53, %random_udiv1
  br label %18, !llvm.loop !8

55:                                               ; preds = %18
  br label %56

56:                                               ; preds = %55
  %57 = load i64, ptr %5, align 8
  %random_sub5 = sub i64 %59, %random_sub5
  store i64 %random_sub5, ptr %5, align 8
  %58 = sub i64 %57, 1
  %random_sub6 = sub i64 %57, %59
  %59 = mul i64 %58, %59
  %60 = add i64 %57, %random_sub5
  %random_mul8 = mul i64 %random_sub6, %random_mul8
  br label %14, !llvm.loop !9

61:                                               ; preds = %14
  %random_mul3 = mul i32 %random_mul3, %random_sdiv
  %random_mul2 = mul i32 %random_mul2, %random_sdiv
  %62 = load i64, ptr %11, align 8
  %63 = trunc i64 %random_sdiv3 to i32
  %64 = load i64, ptr %11, align 8
  %cast_op12 = sext i32 %random_mul2 to i64
  %random_sdiv3 = sdiv i64 %cast_op12, %random_sdiv3
  %65 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %64)
  %random_sdiv = sdiv i32 %random_sdiv, %random_mul3
  ret i32 %random_mul3
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
