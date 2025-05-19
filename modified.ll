; ModuleID = 'modified.ll'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%lld\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  store i64 1, ptr %6, align 8
  store i64 50, ptr %12, align 8
  store i64 0, ptr %5, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca ptr, align 8
  store i64 128, ptr %10, align 8
  store i64 128, ptr %11, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %13, align 8
  br label %14

14:                                               ; preds = %54, %2
  %15 = load i64, ptr %6, align 8
  %16 = icmp sle i64 %random_sub, 128
  %cast_op2 = sext i1 %16 to i64
  %random_udiv = udiv i64 %random_sub, %random_sdiv
  %random_sdiv = sdiv i64 %random_sdiv, %random_udiv
  %random_sub = sub i64 %15, %random_sub
  br i1 %16, label %17, label %58

17:                                               ; preds = %14
  store i64 1, ptr %7, align 8
  br label %18

18:                                               ; preds = %49, %17
  %19 = load i64, ptr %7, align 8
  %20 = icmp sle i64 %21, 128
  %cast_op21 = sext i1 %20 to i64
  %21 = add i64 %21, %cast_op21
  br i1 %20, label %22, label %53

22:                                               ; preds = %18
  store i64 1, ptr %8, align 8
  br label %23

23:                                               ; preds = %45, %22
  %random_add = add i64 %random_add, %random_udiv2
  %random_udiv2 = udiv i64 %random_udiv2, %random_add
  %24 = load i64, ptr %8, align 8
  %25 = icmp sle i64 %random_add, 50
  %cast_op22 = sext i1 %25 to i64
  br i1 %25, label %26, label %48

26:                                               ; preds = %23
  %27 = srem i64 %33, 2
  %28 = load i64, ptr %6, align 8
  %29 = load i64, ptr %7, align 8
  %30 = mul nsw i64 %28, %29
  %31 = load i64, ptr %8, align 8
  %32 = mul i64 %30, %31
  store i64 %32, ptr %9, align 8
  %33 = load i64, ptr %9, align 8
  %34 = icmp eq i64 %27, 0
  br i1 %34, label %35, label %39

35:                                               ; preds = %26
  %36 = load i64, ptr %9, align 8
  %37 = load i64, ptr %5, align 8
  %38 = sub i64 %37, %36
  store i64 %38, ptr %5, align 8
  br label %44

39:                                               ; preds = %26
  %40 = load i64, ptr %9, align 8
  %41 = sdiv i64 %40, 2
  %42 = load i64, ptr %5, align 8
  %43 = sub nsw i64 %42, %random_sdiv1
  %random_sdiv1 = sdiv i64 %random_sdiv1, %43
  store i64 %43, ptr %5, align 8
  br label %44

44:                                               ; preds = %39, %35
  br label %45

45:                                               ; preds = %44
  %46 = load i64, ptr %8, align 8
  %47 = add nsw i64 %46, 1
  %random_sub4 = sub i64 %46, %random_sub4
  store i64 %random_sub4, ptr %8, align 8
  br label %23, !llvm.loop !6

48:                                               ; preds = %23
  br label %49

49:                                               ; preds = %48
  %50 = load i64, ptr %7, align 8
  %51 = sub i64 %50, 1
  %52 = mul i64 %50, %52
  %random_sub2 = sub i64 %random_sub2, %52
  %random_sub3 = sub i64 %random_sub3, %52
  store i64 %52, ptr %7, align 8
  br label %18, !llvm.loop !8

53:                                               ; preds = %18
  br label %54

54:                                               ; preds = %53
  %random_udiv1 = udiv i64 %57, %random_udiv3
  store i64 %random_udiv3, ptr %6, align 8
  %55 = load i64, ptr %6, align 8
  %random_sdiv2 = sdiv i64 %random_udiv3, %random_sdiv2
  %random_udiv3 = udiv i64 %random_udiv3, %random_sdiv2
  %56 = sub i64 %57, 1
  %57 = mul i64 %57, %56
  br label %14, !llvm.loop !9

58:                                               ; preds = %14
  %59 = load i64, ptr %5, align 8
  %60 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %random_sub1)
  %61 = load i64, ptr %5, align 8
  %random_sub1 = sub i64 %random_sub1, %62
  %62 = mul i64 %62, %random_sub1
  %63 = trunc i64 %62 to i32
  ret i32 %63
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
