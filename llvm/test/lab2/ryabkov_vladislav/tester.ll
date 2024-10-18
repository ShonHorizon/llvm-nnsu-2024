; RUN: opt -load-pass-plugin %llvmshlibdir/RyabkovInstrumentFunctions%pluginext -passes=instrumentation-functions -S %s | FileCheck %s
define dso_local void @_Z16instrument_startv() #0 {
  ret void
}

define dso_local void @_Z14instrument_endv() #0 {
  ret void
}

define dso_local void @_Z5basicv() #0 {
  ret void
}

; CHECK-LABEL: @_Z5basicv
; CHECK: call void @instrument_start()
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret void

define dso_local noundef i32 @_Z6squarei(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = mul nsw i32 %3, %4
  ret i32 %5
}

; CHECK-LABEL: @_Z6squarei
; CHECK: call void @instrument_start()
; CHECK-NEXT: %2 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %2, align 4
; CHECK-NEXT: %3 = load i32, ptr %2, align 4
; CHECK-NEXT: %4 = load i32, ptr %2, align 4
; CHECK-NEXT: %5 = mul nsw i32 %3, %4
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 %5


define dso_local noundef i32 @_Z3minii(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  %6 = load i32, ptr %4, align 4
  %7 = load i32, ptr %5, align 4
  %8 = icmp sle i32 %6, %7
  br i1 %8, label %9, label %11

9:                                                ; preds = %2
  %10 = load i32, ptr %5, align 4
  store i32 %10, ptr %3, align 4
  br label %13

11:                                               ; preds = %2
  %12 = load i32, ptr %4, align 4
  store i32 %12, ptr %3, align 4
  br label %13

13:                                               ; preds = %11, %9
  %14 = load i32, ptr %3, align 4
  ret i32 %14
}

; CHECK-LABEL: @_Z3minii
; CHECK: call void @instrument_start()
; CHECK-NEXT: %3 = alloca i32, align 4
; CHECK-NEXT: %4 = alloca i32, align 4
; CHECK-NEXT: %5 = alloca i32, align 4
; CHECK-NEXT: store i32 %0, ptr %4, align 4
; CHECK-NEXT: store i32 %1, ptr %5, align 4
; CHECK-NEXT: %6 = load i32, ptr %4, align 4
; CHECK-NEXT: %7 = load i32, ptr %5, align 4
; CHECK-NEXT: %8 = icmp sle i32 %6, %7
; CHECK-NEXT: br i1 %8, label %9, label %11
; CHECK:9:                                                ; preds = %2
; CHECK-NEXT: %10 = load i32, ptr %5, align 4
; CHECK-NEXT: store i32 %10, ptr %3, align 4
; CHECK-NEXT: br label %13
; CHECK:11:                                               ; preds = %2
; CHECK-NEXT: %12 = load i32, ptr %4, align 4
; CHECK-NEXT: store i32 %12, ptr %3, align 4
; CHECK-NEXT: br label %13
; CHECK:13:                                               ; preds = %11, %9
; CHECK-NEXT: %14 = load i32, ptr %3, align 4
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i32 %14

define dso_local noundef signext i8 @_Z15sybmolCodeASCIIc(i8 noundef signext %0) #0 {
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  store i8 %0, ptr %3, align 1
  %4 = load i8, ptr %3, align 1
  %5 = sext i8 %4 to i32
  switch i32 %5, label %9 [
    i32 65, label %6
    i32 66, label %7
    i32 67, label %8
  ]

6:                                                ; preds = %1
  store i8 65, ptr %2, align 1
  br label %10

7:                                                ; preds = %1
  store i8 66, ptr %2, align 1
  br label %10

8:                                                ; preds = %1
  store i8 67, ptr %2, align 1
  br label %10

9:                                                ; preds = %1
  store i8 10, ptr %2, align 1
  br label %10

10:                                               ; preds = %9, %8, %7, %6
  %11 = load i8, ptr %2, align 1
  ret i8 %11
}

; CHECK-LABEL: @_Z15sybmolCodeASCIIc
; CHECK-NEXT: call void @instrument_start()
; CHECK-NEXT: %2 = alloca i8, align 1
; CHECK-NEXT: %3 = alloca i8, align 1
; CHECK-NEXT: store i8 %0, ptr %3, align 1
; CHECK-NEXT: %4 = load i8, ptr %3, align 1
; CHECK-NEXT: %5 = sext i8 %4 to i32
; CHECK-NEXT: switch i32 %5, label %9 [
; CHECK-NEXT:   i32 65, label %6
; CHECK-NEXT:   i32 66, label %7
; CHECK-NEXT:   i32 67, label %8
; CHECK-NEXT: ]
; CHECK:6:                                                ; preds = %1
; CHECK-NEXT: store i8 65, ptr %2, align 1
; CHECK-NEXT: br label %10
; CHECK:7:                                                ; preds = %1
; CHECK-NEXT: store i8 66, ptr %2, align 1
; CHECK-NEXT: br label %10
; CHECK:8:                                                ; preds = %1
; CHECK-NEXT: store i8 67, ptr %2, align 1
; CHECK-NEXT: br label %10
; CHECK:9:                                                ; preds = %1
; CHECK-NEXT: store i8 10, ptr %2, align 1
; CHECK-NEXT: br label %10
; CHECK:10:                                               ; preds = %9, %8, %7, %6
; CHECK-NEXT: %11 = load i8, ptr %2, align 1
; CHECK-NEXT: call void @instrument_end()
; CHECK-NEXT: ret i8 %11


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
