; RUN: opt -load-pass-plugin %llvmshlibdir/MGKInstrumentFunctions%pluginext -passes=mgk_func_wrapper -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z5Emptyv() #0 {
entry:
  ; CHECK: call void @start_instrument()
  ; CHECK-NEXT: call void @end_instrument()
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z4foo1ii(i32 noundef %a, i32 noundef %b) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add

  ; CHECK: call void @start_instrument()
  ; CHECK-NEXT: %a.addr = alloca i32, align 4
  ; CHECK-NEXT: %b.addr = alloca i32, align 4
  ; CHECK-NEXT: store i32 %a, ptr %a.addr, align 4
  ; CHECK-NEXT: store i32 %b, ptr %b.addr, align 4
  ; CHECK-NEXT: %0 = load i32, ptr %a.addr, align 4
  ; CHECK-NEXT: %1 = load i32, ptr %b.addr, align 4
  ; CHECK-NEXT: %add = add nsw i32 %0, %1
  ; CHECK-NEXT: call void @end_instrument()
  ; CHECK-NEXT: ret i32 %add
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3sumi(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %res = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  store i32 0, ptr %res, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

  ; CHECK: call void @start_instrument()
  ; CHECK-NEXT: %n.addr = alloca i32, align 4
  ; CHECK-NEXT: %res = alloca i32, align 4
  ; CHECK-NEXT: %i = alloca i32, align 4
  ; CHECK-NEXT: store i32 %n, ptr %n.addr, align 4
  ; CHECK-NEXT: store i32 0, ptr %res, align 4
  ; CHECK-NEXT: store i32 0, ptr %i, align 4
  ; CHECK-NEXT: br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %1 = load i32, ptr %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr %i, align 4
  %mul = mul nsw i32 2, %2
  %3 = load i32, ptr %res, align 4
  %add = add nsw i32 %3, %mul
  store i32 %add, ptr %res, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, ptr %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32, ptr %res, align 4
  ret i32 %5

  ; CHECK: %5 = load i32, ptr %res, align 4
  ; CHECK-NEXT: call void @end_instrument()
  ; CHECK-NEXT: ret i32 %5
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3cmpii(i32 noundef %a, i32 noundef %b) #0 {
entry:
  %retval = alloca i32, align 4
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp sgt i32 %0, %1
  br i1 %cmp, label %if.then, label %if.else

  ; CHECK: call void @start_instrument()
  ; CHECK-NEXT: %retval = alloca i32, align 4
  ; CHECK-NEXT: %a.addr = alloca i32, align 4
  ; CHECK-NEXT: %b.addr = alloca i32, align 4
  ; CHECK-NEXT: store i32 %a, ptr %a.addr, align 4
  ; CHECK-NEXT: store i32 %b, ptr %b.addr, align 4
  ; CHECK-NEXT: %0 = load i32, ptr %a.addr, align 4
  ; CHECK-NEXT: %1 = load i32, ptr %b.addr, align 4
  ; CHECK-NEXT: %cmp = icmp sgt i32 %0, %1
  ; CHECK-NEXT: br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 1, ptr %retval, align 4
  br label %return

if.else:                                          ; preds = %entry
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.else, %if.then
  %2 = load i32, ptr %retval, align 4
  ret i32 %2

  ; CHECK: %2 = load i32, ptr %retval, align 4
  ; CHECK-NEXT: call void @end_instrument()
  ; CHECK-NEXT: ret i32 %2
}

define dso_local noundef i32 @_Z4foo2ii(i32 noundef %a, i32 noundef %b) #0 {
entry:
  call void @start_instrument()
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  call void @end_instrument()
  ret i32 %add

  ; CHECK-LABEL: @_Z4foo2ii
  ; CHECK-NEXT: entry:
  ; CHECK-NEXT: call void @start_instrument()
  ; CHECK-NEXT: %a.addr = alloca i32, align 4
  ; CHECK-NEXT: %b.addr = alloca i32, align 4
  ; CHECK-NEXT: store i32 %a, ptr %a.addr, align 4
  ; CHECK-NEXT: store i32 %b, ptr %b.addr, align 4
  ; CHECK-NEXT: %0 = load i32, ptr %a.addr, align 4
  ; CHECK-NEXT: %1 = load i32, ptr %b.addr, align 4
  ; CHECK-NEXT: %add = add nsw i32 %0, %1
  ; CHECK-NEXT: call void @end_instrument()
  ; CHECK-NEXT: ret i32 %add
}

declare void @start_instrument()

declare void @end_instrument()

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}