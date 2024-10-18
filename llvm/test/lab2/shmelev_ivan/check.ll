; RUN: opt -load-pass-plugin %llvmshlibdir/ShmelevInstrumnetF%pluginext -passes=instrument_functions -S %s | FileCheck %s
define dso_local void @_Z16instrument_startv() #0 {
  ret void
}

define dso_local void @_Z14instrument_endv() #0 {
  ret void
}

; void plain() {
;     return;
; }

define dso_local void @_Z5plainv() #0 {
  ret void
}

; CHECK-LABEL: @_Z5plainv
; CHECK-NEXT: call void @instrument_start()
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret void


; int add(int a, int b) {
;     return a + b;
; }

define dso_local noundef i32 @_Z3addii(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %5 = load i32, ptr %3, align 4
  %6 = load i32, ptr %4, align 4
  %7 = add nsw i32 %5, %6
  ret i32 %7
}

; CHECK-LABEL: @_Z3addii
; CHECK: call void @instrument_start()
; CHECK-NEXT: %3 = alloca i32, align 4
; CHECK-NEXT: %4 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %3, align 4
; CHECK-NEXT: store i32 %1, ptr %4, align 4
; CHECK-NEXT: %5 = load i32, ptr %3, align 4
; CHECK-NEXT: %6 = load i32, ptr %4, align 4
; CHECK-NEXT: %7 = add nsw i32 %5, %6
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 %7


; char modified(int a) {
;     a *= 8;
;     a %= 10;
; 
;     if (a == 1) {
;         return 'Y';
;     }
;
;     return 'N';
; }

define dso_local noundef signext i8 @_Z8modifiedi(i32 noundef %0) #0 {
  %2 = alloca i8, align 1
  %3 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  %4 = load i32, ptr %3, align 4
  %5 = mul nsw i32 %4, 8
  store i32 %5, ptr %3, align 4
  %6 = load i32, ptr %3, align 4
  %7 = srem i32 %6, 10
  store i32 %7, ptr %3, align 4
  %8 = load i32, ptr %3, align 4
  %9 = icmp eq i32 %8, 1
  br i1 %9, label %10, label %11

10:                                               ; preds = %1
  store i8 89, ptr %2, align 1
  br label %12

11:                                               ; preds = %1
  store i8 78, ptr %2, align 1
  br label %12

12:                                               ; preds = %11, %10
  %13 = load i8, ptr %2, align 1
  ret i8 %13
}

; CHECK-LABEL: @_Z8modifiedi
; CHECK: call void @instrument_start()
; CHECK-NEXT: %2 = alloca i8, align 1
; CHECK-NEXT: %3 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %3, align 4
; CHECK-NEXT: %4 = load i32, ptr %3, align 4
; CHECK-NEXT: %5 = mul nsw i32 %4, 8
; CHECK-NEXT: store i32 %5, ptr %3, align 4
; CHECK-NEXT: %6 = load i32, ptr %3, align 4
; CHECK-NEXT: %7 = srem i32 %6, 10
; CHECK-NEXT: store i32 %7, ptr %3, align 4
; CHECK-NEXT: %8 = load i32, ptr %3, align 4
; CHECK-NEXT: %9 = icmp eq i32 %8, 1
; CHECK-NEXT: br i1 %9, label %10, label %11
; CHECK: 10:                                               ; preds = %1
; CHECK-NEXT: store i8 89, ptr %2, align 1
; CHECK-NEXT: br label %12
; CHECK: 11:                                               ; preds = %1
; CHECK-NEXT: store i8 78, ptr %2, align 1
; CHECK-NEXT: br label %12
; CHECK: 12:                                               ; preds = %11, %10
; CHECK-NEXT: %13 = load i8, ptr %2, align 1
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i8 %13

; int max(int x, int y) {
;     if (x >= y) {
;         return x;
;     }
; 
;     return y;
; }


define dso_local noundef i32 @_Z3maxii(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  %6 = load i32, ptr %4, align 4
  %7 = load i32, ptr %5, align 4
  %8 = icmp sge i32 %6, %7
  br i1 %8, label %9, label %11

9:                                                ; preds = %2
  %10 = load i32, ptr %4, align 4
  store i32 %10, ptr %3, align 4
  br label %13

11:                                               ; preds = %2
  %12 = load i32, ptr %5, align 4
  store i32 %12, ptr %3, align 4
  br label %13

13:                                               ; preds = %11, %9
  %14 = load i32, ptr %3, align 4
  ret i32 %14
}

; CHECK-LABEL: @_Z3maxii
; CHECK: call void @instrument_start()
; CHECK-NEXT: %3 = alloca i32, align 4
; CHECK-NEXT: %4 = alloca i32, align 4
; CHECK-NEXT: %5 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %4, align 4
; CHECK-NEXT: store i32 %1, ptr %5, align 4
; CHECK-NEXT: %6 = load i32, ptr %4, align 4
; CHECK-NEXT: %7 = load i32, ptr %5, align 4
; CHECK-NEXT: %8 = icmp sge i32 %6, %7
; CHECK-NEXT: br i1 %8, label %9, label %11
; CHECK: 9:                                                ; preds = %2
; CHECK-NEXT: %10 = load i32, ptr %4, align 4
; CHECK-NEXT: store i32 %10, ptr %3, align 4
; CHECK-NEXT: br label %13
; CHECK: 11:                                               ; preds = %2
; CHECK-NEXT: %12 = load i32, ptr %5, align 4
; CHECK-NEXT: store i32 %12, ptr %3, align 4
; CHECK-NEXT: br label %13
; CHECK: 13:                                               ; preds = %11, %9
; CHECK-NEXT: %14 = load i32, ptr %3, align 4
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 %14

define i32 @multi_ret(i32 %x) {
entry:
    %is_zero = icmp eq i32 %x, 0
    br i1 %is_zero, label %return_zero, label %check_negative

check_negative:
    %is_negative = icmp slt i32 %x, 0
    br i1 %is_negative, label %return_negative, label %return_positive

return_zero:
    ret i32 0

return_negative:
    ret i32 -1

return_positive:
    ret i32 1
}

; CHECK-LABEL: multi_ret
; CHECK: entry:
; CHECK-NEXT: call void @instrument_start()
; CHECK-NEXT: %is_zero = icmp eq i32 %x, 0
; CHECK-NEXT: br i1 %is_zero, label %return_zero, label %check_negative
; CHECK: check_negative:                                   ; preds = %entry
; CHECK-NEXT: %is_negative = icmp slt i32 %x, 0
; CHECK-NEXT: br i1 %is_negative, label %return_negative, label %return_positive
; CHECK: return_zero:                                      ; preds = %entry
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 0
; CHECK: return_negative:                                  ; preds = %check_negative
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 -1
; CHECK:return_positive:                                  ; preds = %check_negative
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 1