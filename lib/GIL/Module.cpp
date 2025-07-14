#include "Module.hpp"

namespace glu::gil {

Function *Module::addFunction(Function *fn)
{
    _functions.push_back(fn);
    return &_functions.back();
};

Function *Module::getFunction(llvm::StringRef name)
{
    for (auto &f : _functions) {
        if (f.getName() == name)
            return &f;
    }
    return nullptr;
};

void Module::deleteFunction(Function *f)
{
    _functions.remove(f);
};

} // end namespace glu::gil
