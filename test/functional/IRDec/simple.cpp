//
// RUN: clang++ -g -c -emit-llvm %s -o %t.bc
// RUN: gluc %t.bc -print-interface | FileCheck -v %s
//

#include <cstddef>

class PizzaManager {
public:
    size_t getPizzaCount() const;
};

// CHECK: @linkage_name("{{.*}}")
// CHECK-SAME: public func getPizzaCount({{.*}}: *PizzaManager) -> UInt64;
size_t PizzaManager::getPizzaCount() const
{
    return 42;
}
