// RUN: split-file %s %t
// RUN: %clang_cc1 -load %llvmshlibdir/printNamesPluginShmelev%pluginext -plugin print-names-plugin-shmelev %t/source.cpp 2>&1 | FileCheck %t/source.cpp

//--- source.cpp

class Simple {};

// CHECK: Simple

class House {
  public:
    static int a;
    int b;

  private:
    int h;
    double t;

};

// CHECK: House
// CHECK-NEXT: |_a
// CHECK-NEXT: |_b
// CHECK-NEXT: |_h
// CHECK-NEXT: |_t

struct Ball {

    int q;
    double p;
    char j;
    static int k;

};

// CHECK: Ball
// CHECK-NEXT: |_q
// CHECK-NEXT: |_p
// CHECK-NEXT: |_j
// CHECK-NEXT: |_k

class Car {
  public:
    int speed;

  private:
    int price;
    double weight;
    static int year;

    class BMW {
      public:
        static int count;
      private:
        char c;
    };
};

// CHECK: Car
// CHECK-NEXT: |_speed
// CHECK-NEXT: |_price
// CHECK-NEXT: |_weight
// CHECK-NEXT: |_year
// CHECK-NEXT: BMW
// CHECK-NEXT: |_count
// CHECK-NEXT: |_c


class Good {
  char x;
};

// CHECK: Good
// CHECK-NEXT: |_x

template<typename T>
class Bear {
  private:
   T m;
};

// CHECK: Bear
// CHECK-NEXT: |_m

// RUN: %clang_cc1 -load %llvmshlibdir/printNamesPluginShmelev%pluginext -plugin print-names-plugin-shmelev -plugin-arg-print-names-plugin-shmelev help 2>&1 %t/source_help.cpp | FileCheck %t/source_help.cpp --check-prefix=HELP

//--- source_help.cpp

// HELP: This plugin outputs the names of all classes/structures and their fields.
// HELP-NOT: |_