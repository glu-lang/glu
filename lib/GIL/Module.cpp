#include "Module.hpp"

namespace glu::gil {

Function *Module::addFunction(Function *fn)
{
    _functions.push_back(fn);
    return &_functions.back();
};

void Module::deleteFunction(Function *f)
{
    _functions.remove(f);
};

} // end namespace glu::gil
