#include "Module.hpp"

namespace glu::gil {

Function *Module::addFunction(std::string name, glu::types::FunctionTy *type)
{
    _functions.push_back(new Function(name, type));
    return &_functions.back();
};

Function const *Module::getFunction(std::string name) const
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
