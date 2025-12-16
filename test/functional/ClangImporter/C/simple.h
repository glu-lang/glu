//
// RUN: gluc %s -print-interface | FileCheck -v %s
//

#include <stddef.h>
#include <stdio.h>

// CHECK: public enum PizzaSize : UInt32 {
enum PizzaSize {
    // CHECK-NEXT: public Small,
    Small,
    // CHECK-NEXT: public Medium,
    Medium,
    // CHECK-NEXT: public Large
    Large
};

// CHECK: public struct Pizza {
struct Pizza {
    // CHECK-NEXT: public size: PizzaSize,
    enum PizzaSize size;
    // CHECK-NEXT: public name: *Char,
    char const *name;
    // CHECK-NEXT: public price: Float64
    double price;
};

// CHECK: @no_mangling public func getPizzaCount() -> UInt64;
size_t getPizzaCount(void);

// CHECK: @no_mangling public func createFavoritePizza(size: PizzaSize)
// CHECK-SAME: -> Pizza;
struct Pizza createFavoritePizza(enum PizzaSize size);

// CHECK: @no_mangling public func getCC() -> *Char;
char const *getCC(void);
