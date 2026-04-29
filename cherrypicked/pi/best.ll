; ModuleID = 'temp.ll'
source_filename = "benchmarks/pi/pi.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"benchmarks/pi/pi.c\00", align 1
@.str.2 = private unnamed_addr constant [14 x i8] c"%s/output.txt\00", align 1
@.str.3 = private unnamed_addr constant [2 x i8] c"w\00", align 1

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
  %39 = load float, ptr %21, align 4
  %40 = fpext float %39 to double
  %41 = fcmp ole double %40, 1.000000e+00
  %42 = mul nsw i64 74383, %38
  %43 = sub nsw i64 %36, %42
  store i64 %43, ptr %25, align 8
  %44 = load i64, ptr %25, align 8
  %45 = sitofp i64 %44 to float
  %46 = fpext float %45 to double
  %47 = fdiv double %46, 7.438300e+04
  %48 = fptrunc double %47 to float
  store float %48, ptr %19, align 4
  %49 = load float, ptr %17, align 4
  %50 = load float, ptr %16, align 4
  %51 = fmul float %49, %50
  store float %51, ptr %23, align 4
  %52 = load float, ptr %23, align 4
  %53 = load float, ptr %18, align 4
  %54 = load float, ptr %23, align 4
  %55 = load float, ptr %18, align 4
  %56 = fdiv float %54, %55
  %57 = fptosi float %56 to i64
  %58 = sitofp i64 %57 to float
  %59 = fneg float %53
  %60 = call float @llvm.fmuladd.f32(float %59, float %58, float %52)
  store float %60, ptr %16, align 4
  %61 = load float, ptr %16, align 4
  %62 = load float, ptr %18, align 4
  %63 = fdiv float %61, %62
  store float %63, ptr %20, align 4
  %64 = load float, ptr %19, align 4
  %65 = load float, ptr %19, align 4
  %66 = load float, ptr %20, align 4
  %67 = load float, ptr %20, align 4
  %68 = fmul float %66, %67
  %69 = call float @llvm.fmuladd.f32(float %64, float %65, float %68)
  store float %69, ptr %21, align 4
  call void @myadd(ptr noundef %15, ptr noundef %21)
  br i1 %41, label %70, label %73

70:                                               ; preds = %33
  %71 = load i64, ptr %24, align 8
  %72 = add nsw i64 %71, 1
  store i64 %72, ptr %24, align 8
  %regCompare = fcmp oeq float %48, %54
  br i1 %regCompare, label %newblock852074, label %73

73:                                               ; preds = %newblock852074, %70, %33
  br label %74

74:                                               ; preds = %73
  %75 = load i64, ptr %27, align 8
  %76 = add nsw i64 %75, 1
  store i64 %76, ptr %27, align 8
  br label %29, !llvm.loop !5

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

newblock852074:                                   ; preds = %70
  br label %73
}

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
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
  %20 = alloca [256 x i8], align 16
  %21 = alloca ptr, align 8
  %22 = alloca [512 x i8], align 16
  %23 = alloca ptr, align 8
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
  %24 = load float, ptr %6, align 4
  %25 = load float, ptr %7, align 4
  %26 = load float, ptr %8, align 4
  %27 = load float, ptr %9, align 4
  %28 = load float, ptr %10, align 4
  %29 = load float, ptr %11, align 4
  %30 = load float, ptr %12, align 4
  %31 = load float, ptr %13, align 4
  %32 = load float, ptr %14, align 4
  %33 = load i64, ptr %15, align 8
  %34 = load i64, ptr %16, align 8
  %35 = load i64, ptr %17, align 8
  %36 = load i64, ptr %18, align 8
  %37 = load i64, ptr %19, align 8
  %38 = call float @computation(float noundef %24, float noundef %25, float noundef %26, float noundef %27, float noundef %28, float noundef %29, float noundef %30, float noundef %31, float noundef %32, i64 noundef %33, i64 noundef %34, i64 noundef %35, i64 noundef %36, i64 noundef %37)
  store float %38, ptr %13, align 4
  %39 = load float, ptr %13, align 4
  %40 = fpext float %39 to double
  %41 = call i32 (ptr, ...) @printf(ptr noundef @.str, double noundef %40)
  %42 = getelementptr inbounds [256 x i8], ptr %20, i64 0, i64 0
  %43 = call ptr @strcpy(ptr noundef %42, ptr noundef @.str.1) #4
  %44 = getelementptr inbounds [256 x i8], ptr %20, i64 0, i64 0
  %45 = call ptr @dirname(ptr noundef %44) #4
  store ptr %45, ptr %21, align 8
  %46 = getelementptr inbounds [512 x i8], ptr %22, i64 0, i64 0
  %47 = load ptr, ptr %21, align 8
  %48 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef %46, i64 noundef 512, ptr noundef @.str.2, ptr noundef %47) #4
  %49 = getelementptr inbounds [512 x i8], ptr %22, i64 0, i64 0
  %50 = call noalias ptr @fopen(ptr noundef %49, ptr noundef @.str.3)
  store ptr %50, ptr %23, align 8
  %51 = load ptr, ptr %23, align 8
  %52 = load float, ptr %13, align 4
  %53 = fpext float %52 to double
  %54 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %51, ptr noundef @.str, double noundef %53) #4
  %55 = load ptr, ptr %23, align 8
  %56 = call i32 @fclose(ptr noundef %55)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #2

; Function Attrs: nounwind
declare ptr @strcpy(ptr noundef, ptr noundef) #3

; Function Attrs: nounwind
declare ptr @dirname(ptr noundef) #3

; Function Attrs: nounwind
declare i32 @snprintf(ptr noundef, i64 noundef, ptr noundef, ...) #3

declare noalias ptr @fopen(ptr noundef, ptr noundef) #2

; Function Attrs: nounwind
declare i32 @fprintf(ptr noundef, ptr noundef, ...) #3

declare i32 @fclose(ptr noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 8, !"PIC Level", i32 2}
!1 = !{i32 7, !"PIE Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 23.0.0git (https://github.com/llvm/llvm-project.git 120cbbd88bf6b6c723176684a12c64393abe7e95)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
