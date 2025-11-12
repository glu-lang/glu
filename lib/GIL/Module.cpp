#include "Module.hpp"

template <typename ListT> void clearList(ListT &list)
{
    while (!list.empty()) {
        list.erase(list.begin());
    }
}

namespace glu::gil {

Module::~Module()
{
    clearFunctions();
    clearList(_globals);
}

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

void Module::clearFunctions()
{
    clearList(_functions);
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
