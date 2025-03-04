; RUN: opt -load-pass-plugin=%llvmshlibdir/ZawadaLoopPlugin%shlibext -passes=loop_func -S %s | FileCheck %s

; int sum()
; {
;     int sum = 0;
;     for (int i = 0; i < 10; i++)
;     {
;         sum += i;
;     }
;     return sum;
; }

define dso_local i32 @sum() #0 {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  store i32 0, ptr %i, align 4
; CHECK: call void @loop_start()
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %2 = load i32, ptr %sum, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
; CHECK: call void @loop_end()
  %4 = load i32, ptr %sum, align 4
  ret i32 %4
}

; int dif(int a, int b) {
; 	while (a < b) {
; 		a -= b;
; 	}
; 	return a;
; } 

define dso_local i32 @dif(i32 noundef %a, i32 noundef %b) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
; CHECK: call void @loop_start()
  br label %while.cond

while.cond:                                        ; preds = %while.body, %entry
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %while.body, label %while.end

while.body:                                        ; preds = %while.cond
  %2 = load i32, ptr %a.addr, align 4
  %3 = load i32, ptr %b.addr, align 4
  %sub = sub nsw i32 %2, %3
  store i32 %sub, ptr %a.addr, align 4
  br label %while.cond

while.end:                                         ; preds = %while.cond
; CHECK: call void @loop_end()
  %4 = load i32, ptr %a.addr, align 4
  ret i32 %4
}

; void funWithoutLoops(int a, int b) {
; 	if (a > b) {
; 		return true;
; 	} else {
; 		return false;
; 	}
; }

; CHECK-LABEL:  @funWithoutLoops
; CHECK-NOT: call void @loop_start()
; CHECK-NOT: call void @loop_end()
; CHECK: ret void

define dso_local void @funWithoutLoops(i32 %a, i32 %b) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32, i32* %a.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  %cmp = icmp sgt i32 %0, %1
  br i1 %cmp, label %if.true, label %if.false

if.true:                                           ; preds = %entry
  ret void

if.false:                                          ; preds = %entry
  ret void
}

; int sumWithLoops() {
; 	int sum = 0;
; 	loop_start();
; 	for (int i = 0; i < 10; i++) {
; 		sum += i;
; 	}
; 	loop_end();
; 	return sum;
; }

declare void @loop_start()
declare void @loop_end()
define dso_local i32 @sumWithLoops() #0 {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %sum, align 4
; CHECK: call void @loop_start()
; CHECK-NEXT: store i32 0, ptr %i, align 4
; CHECK-NEXT: br label %for.cond
  call void @loop_start()
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %2 = load i32, ptr %sum, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
; CHECK: call void @loop_end()
; CHECK-NEXT: %4 = load i32, ptr %sum, align 4
; CHECK-NEXT: ret i32 %4
  call void @loop_end()
  %4 = load i32, ptr %sum, align 4
  ret i32 %4
}
