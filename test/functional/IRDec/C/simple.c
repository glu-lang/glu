//
// RUN: clang -g -c -emit-llvm %s -o %t.bc
// RUN: gluc %t.bc -print-interface | FileCheck -v %s
// RUN: gluc %s -print-interface | FileCheck -v %s
//

#include <stddef.h>

// CHECK: @no_mangling public func getPizzaCount() -> UInt64;
size_t getPizzaCount()
{
    return 42;
}

// CHECK: @no_mangling public func getCC() -> *Char;
char const *getCC()
{
    return "Clang";
}
