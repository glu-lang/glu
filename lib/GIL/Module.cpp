#include "Module.hpp"

namespace glu::gil {

Function *Module::addFunction(Function *fn)
{
    _functions.push_back(fn);
    return &_functions.back();
}

Global *Module::addGlobal(Global *global)
{
    _globals.push_back(global);
    return &_globals.back();
}

void Module::deleteFunction(Function *f)
{
    if (!f) {
        return;
    }
    _functions.erase(f->getIterator());
}

gil::Function *Module::getFunctionByDecl(ast::FunctionDecl *decl)
{
    for (auto &fn : _functions) {
        if (fn.getDecl() == decl) {
            return &fn;
        }
    }
    return nullptr;
}

} // end namespace glu::gil
