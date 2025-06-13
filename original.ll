; ModuleID = 'original_code.c'
source_filename = "original_code.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @myadd(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load float, ptr %5, align 4
  %7 = load ptr, ptr %4, align 8
  %8 = load float, ptr %7, align 4
  %9 = fadd float %6, %8
  %10 = load ptr, ptr %3, align 8
  store float %9, ptr %10, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @computation(float noundef %0, float noundef %1, float noundef %2, float noundef %3, float noundef %4, float noundef %5, float noundef %6, float noundef %7, float noundef %8, i64 noundef %9, i64 noundef %10, i64 noundef %11, i64 noundef %12, i64 noundef %13) #0 section "OPTIMIZABLE_SECTION" {
  %15 = alloca float, align 4
  %16 = alloca float, align 4
  %17 = alloca float, align 4
  %18 = alloca float, align 4
  %19 = alloca float, align 4
  %20 = alloca float, align 4
  %21 = alloca float, align 4
  %22 = alloca float, align 4
  %23 = alloca float, align 4
  %24 = alloca i64, align 8
  %25 = alloca i64, align 8
  %26 = alloca i64, align 8
  %27 = alloca i64, align 8
  %28 = alloca i64, align 8
  store float %0, ptr %15, align 4
  store float %1, ptr %16, align 4
  store float %2, ptr %17, align 4
  store float %3, ptr %18, align 4
  store float %4, ptr %19, align 4
  store float %5, ptr %20, align 4
  store float %6, ptr %21, align 4
  store float %7, ptr %22, align 4
  store float %8, ptr %23, align 4
  store i64 %9, ptr %24, align 8
  store i64 %10, ptr %25, align 8
  store i64 %11, ptr %26, align 8
  store i64 %12, ptr %27, align 8
  store i64 %13, ptr %28, align 8
  store i64 1, ptr %27, align 8
  br label %29

29:                                               ; preds = %74, %14
  %30 = load i64, ptr %27, align 8
  %31 = load i64, ptr %26, align 8
  %32 = icmp sle i64 %30, %31
  br i1 %32, label %33, label %77

33:                                               ; preds = %29
  %34 = load i64, ptr %25, align 8
  %35 = mul nsw i64 27611, %34
  store i64 %35, ptr %28, align 8
  %36 = load i64, ptr %28, align 8
  %37 = load i64, ptr %28, align 8
  %38 = sdiv i64 %37, 74383
  %39 = mul nsw i64 74383, %38
  %40 = sub nsw i64 %36, %39
  store i64 %40, ptr %25, align 8
  %41 = load i64, ptr %25, align 8
  %42 = sitofp i64 %41 to float
  %43 = fpext float %42 to double
  %44 = fdiv double %43, 7.438300e+04
  %45 = fptrunc double %44 to float
  store float %45, ptr %19, align 4
  %46 = load float, ptr %17, align 4
  %47 = load float, ptr %16, align 4
  %48 = fmul float %46, %47
  store float %48, ptr %23, align 4
  %49 = load float, ptr %23, align 4
  %50 = load float, ptr %18, align 4
  %51 = load float, ptr %23, align 4
  %52 = load float, ptr %18, align 4
  %53 = fdiv float %51, %52
  %54 = fptosi float %53 to i64
  %55 = sitofp i64 %54 to float
  %56 = fneg float %50
  %57 = call float @llvm.fmuladd.f32(float %56, float %55, float %49)
  store float %57, ptr %16, align 4
  %58 = load float, ptr %16, align 4
  %59 = load float, ptr %18, align 4
  %60 = fdiv float %58, %59
  store float %60, ptr %20, align 4
  %61 = load float, ptr %19, align 4
  %62 = load float, ptr %19, align 4
  %63 = load float, ptr %20, align 4
  %64 = load float, ptr %20, align 4
  %65 = fmul float %63, %64
  %66 = call float @llvm.fmuladd.f32(float %61, float %62, float %65)
  store float %66, ptr %21, align 4
  call void @myadd(ptr noundef %15, ptr noundef %21)
  %67 = load float, ptr %21, align 4
  %68 = fpext float %67 to double
  %69 = fcmp ole double %68, 1.000000e+00
  br i1 %69, label %70, label %73

70:                                               ; preds = %33
  %71 = load i64, ptr %24, align 8
  %72 = add nsw i64 %71, 1
  store i64 %72, ptr %24, align 8
  br label %73

73:                                               ; preds = %70, %33
  br label %74

74:                                               ; preds = %73
  %75 = load i64, ptr %27, align 8
  %76 = add nsw i64 %75, 1
  store i64 %76, ptr %27, align 8
  br label %29, !llvm.loop !6

77:                                               ; preds = %29
  %78 = load i64, ptr %24, align 8
  %79 = sitofp i64 %78 to float
  %80 = fpext float %79 to double
  %81 = fmul double 4.000000e+00, %80
  %82 = load i64, ptr %26, align 8
  %83 = sitofp i64 %82 to float
  %84 = fpext float %83 to double
  %85 = fdiv double %81, %84
  %86 = fptrunc double %85 to float
  store float %86, ptr %22, align 4
  %87 = load float, ptr %22, align 4
  ret float %87
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.fmuladd.f32(float, float, float) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca float, align 4
  %7 = alloca float, align 4
  %8 = alloca float, align 4
  %9 = alloca float, align 4
  %10 = alloca float, align 4
  %11 = alloca float, align 4
  %12 = alloca float, align 4
  %13 = alloca float, align 4
  %14 = alloca float, align 4
  %15 = alloca i64, align 8
  %16 = alloca i64, align 8
  %17 = alloca i64, align 8
  %18 = alloca i64, align 8
  %19 = alloca i64, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store float 0.000000e+00, ptr %6, align 4
  store i64 1, ptr %15, align 8
  store i64 1907, ptr %16, align 8
  store float 5.813000e+03, ptr %7, align 4
  store float 1.307000e+03, ptr %8, align 4
  store float 5.471000e+03, ptr %9, align 4
  store i64 4000000, ptr %17, align 8
  %20 = load float, ptr %6, align 4
  %21 = load float, ptr %7, align 4
  %22 = load float, ptr %8, align 4
  %23 = load float, ptr %9, align 4
  %24 = load float, ptr %10, align 4
  %25 = load float, ptr %11, align 4
  %26 = load float, ptr %12, align 4
  %27 = load float, ptr %13, align 4
  %28 = load float, ptr %14, align 4
  %29 = load i64, ptr %15, align 8
  %30 = load i64, ptr %16, align 8
  %31 = load i64, ptr %17, align 8
  %32 = load i64, ptr %18, align 8
  %33 = load i64, ptr %19, align 8
  %34 = call float @computation(float noundef %20, float noundef %21, float noundef %22, float noundef %23, float noundef %24, float noundef %25, float noundef %26, float noundef %27, float noundef %28, i64 noundef %29, i64 noundef %30, i64 noundef %31, i64 noundef %32, i64 noundef %33)
  store float %34, ptr %13, align 4
  %35 = load float, ptr %13, align 4
  %36 = fpext float %35 to double
  %37 = call i32 (ptr, ...) @printf(ptr noundef @.str, double noundef %36)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

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
