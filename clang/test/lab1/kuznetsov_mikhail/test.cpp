// RUN: %clang_cc1 -load %llvmshlibdir/PrintikClassPlugin%pluginext -plugin prin-elds %s 2>&1 | FileCheck %s --check-prefix=CHECK1

// CHECK1: Empty
class Empty {};

// CHECK1: MyClass
struct MyClass {
    // CHECK1-NEXT: |_Variable
    int Variable;
};

// CHECK1: Test
struct Test
{
    // CHECK1-NEXT: |_A
    int A;
    // CHECK1-NEXT: |_B
    int B;
};

// CHECK1: TClass
template<typename T> class TClass {
    // CHECK1-NEXT: |_TVar
    T TVar;
};

// CHECK1: ClassWithStaticFields
class ClassWithStaticFields {
    // CHECK1-NEXT: |_StaticField
    static int StaticField;
    // CHECK1-NEXT: |_StaticField2
    static int StaticField2;
    // CHECK1-NEXT: |_Field
    float Field;
};
// RUN: %clang_cc1 -load %llvmshlibdir/PrintikClassPlugin%pluginext -plugin prin-elds -plugin-arg-prin-elds no_fields %s 2>&1 | FileCheck %s --check-prefix=CHECK2

// CHECK2: AnotherClassWithStaticFields
class AnotherClassWithStaticFields {
    // CHECK2-NOT: |_Field1
    static int Field1;
    // CHECK2-NOT: |_Field2
    static int Field2;
    // CHECK2-NOT: |_Field3
    float Field3;
};