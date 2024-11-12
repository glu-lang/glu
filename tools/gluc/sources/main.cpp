#include "GIL/Function.hpp"
#include "GIL/Module.hpp"
#include "IRGen/IRGen.hpp"
#include <iostream>

int main() {
    glu::gil::Module module("test");

    module.addFunction("main", nullptr, {});
    module.addFunction("foo", nullptr, {});
    module.addFunction("bar", nullptr, {});
    std::cout << module.getName() << std::endl;
    for (auto &f : module.getFunctions())
        std::cout << f.getName() << std::endl;
    // module.clearFunctions();
    return 0;
}
