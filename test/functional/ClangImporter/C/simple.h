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
    // CHECK-NEXT: public name: *Char,
    char const *name;
    // CHECK-NEXT: public size: PizzaSize,
    enum PizzaSize size;
    // CHECK-NEXT: public price: Float64
    double price;
};

// CHECK: @no_mangling public func getPizzaCount() -> UInt64;
size_t getPizzaCount(void);

// CHECK: @no_mangling public func createFavoritePizza(size: PizzaSize)
// CHECK-SAME: -> *Pizza;
struct Pizza *createFavoritePizza(enum PizzaSize size);

// CHECK: @no_mangling public func getCC() -> *Char;
char const *getCC(void);

// CHECK: @no_mangling public func headerDefined() -> Int32;
int headerDefined(void)
{
    return 42;
}

static inline int hiddenInlineSecret(void)
{
    return 7;
}

// CHECK-NOT: hiddenInlineSecret
