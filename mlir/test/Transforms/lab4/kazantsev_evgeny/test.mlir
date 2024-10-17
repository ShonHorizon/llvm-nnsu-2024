// RUN: split-file %s %t
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/one.mlir | FileCheck %t/one.mlir
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/two.mlir | FileCheck %t/two.mlir
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/three.mlir | FileCheck %t/three.mlir
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/four.mlir | FileCheck %t/four.mlir
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/five.mlir | FileCheck %t/five.mlir
// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/KazantsevPassDepthMax%shlibext --pass-pipeline="builtin.module(func.func(max-depth-pass))" %t/six.mlir | FileCheck %t/six.mlir

//--- one.mlir
module{
    // CHECK: func.func @one() attributes {maxDepth = 2 : i32} {
    func.func @one() {
        %cond = arith.constant 1 : i1
        %0 = scf.if %cond -> (i1) {
            scf.yield %cond : i1
        } else {
            scf.yield %cond : i1
        }
        func.return
    }    
}

//--- two.mlir
module{
    // CHECK: func.func @two() attributes {maxDepth = 3 : i32} {
    func.func @two() {
        %cond = arith.constant 1 : i1
        %0 = scf.if %cond -> (i1) {
            %1 = scf.if %cond -> (i1) {
                scf.yield %cond : i1
            } else {
                scf.yield %cond : i1
            }
            scf.yield %cond : i1
        } else {
            scf.yield %cond : i1
        }
        func.return
    }
}

//--- three.mlir
module{ 
    // CHECK: func.func @three() attributes {maxDepth = 1 : i32} {
    func.func @three() {
        func.return
    }   
}

//--- four.mlir
func.func @four() {
// CHECK: func.func @four() attributes {maxDepth = 2 : i32}
  %cond = arith.constant 1 : i1
    %0 = scf.if %cond -> (i1) {
        scf.yield %cond : i1
    } else {
        scf.yield %cond : i1
    }
    func.return
}

//--- five.mlir
module{
// CHECK: func.func @five(%arg0: memref<1024xf32>, %arg1: index, %arg2: index, %arg3: index) -> f32 attributes {maxDepth = 2 : i32} {
    func.func @five(%buffer: memref<1024xf32>, %lb: index, %ub: index, %step: index) -> f32 {
        %sum_0 = arith.constant 0.0 : f32
        %sum = scf.for %iv = %lb to %ub step %step iter_args(%sum_iter = %sum_0) -> (f32) {
            %sum_next = arith.addf %sum_iter, %sum_iter : f32
            scf.yield %sum_next : f32
        }
        return %sum : f32
    }
}

//--- six.mlir
module{
    // CHECK: func.func @six() attributes {maxDepth = 3 : i32} {
    func.func @six() {
        %cond = arith.constant 1 : i1
        %0 = scf.if %cond -> (i1) {
            %1 = scf.if %cond -> (i1) {
                %inner_cond = arith.constant 1 : i1
                scf.yield %inner_cond : i1
            } else {
                %inner_cond = arith.constant 0 : i1
                scf.yield %inner_cond : i1
            }
            scf.yield %cond : i1
        } else {
            %inner_cond = arith.constant 0 : i1
            scf.yield %inner_cond : i1
        }
        func.return
    }
}
