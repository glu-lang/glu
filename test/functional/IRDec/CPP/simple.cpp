//
// RUN: clang++ -g -c -emit-llvm %s -o %t.bc
// RUN: gluc %t.bc -print-interface | FileCheck -v %s
// RUN: gluc %s -print-interface | FileCheck -v %s
//

#include <cstddef>

class PizzaManager {
public:
    size_t getPizzaCount() const;
    size_t getPizzaMultiplier() const;
};

// CHECK: @linkage_name("{{.*}}")
// CHECK-SAME: public func getPizzaCount(this: *PizzaManager) -> UInt64;
size_t PizzaManager::getPizzaCount() const
{
    return 42 * getPizzaMultiplier();
}

// CHECK: @linkage_name("{{.*}}") public func getPizzaMultiplier(this:
// *PizzaManager) -> UInt64;
[[gnu::weak]] size_t PizzaManager::getPizzaMultiplier() const
{
    return 1;
}

// CHECK: @linkage_name("{{.*}}") public func getCC() -> *Char;
char const *getCC()
{
    return "Clang++";
}
