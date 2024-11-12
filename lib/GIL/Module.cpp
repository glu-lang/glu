#include "Module.hpp"

namespace glu::gil {

void Module::addFunction(
    std::string name, glu::types::FunctionTy *type,
    std::list<BasicBlock> basicBlocks
)
{
    Function *f = new Function(name, type);

    for (auto &bb : basicBlocks)
        f->addBasicBlockAtEnd(&bb);
    _functions.push_back(std::move(f));
};

Function const *Module::getFunction(std::string name) const
{
    for (auto &f : _functions)
        if (f.getName() == name)
            return &f;
    return nullptr;
};

void Module::deleteFunction(Function *f)
{
    _functions.remove(f);
    delete f;
};

void Module::deleteFunction(std::string name)
{
    for (auto &f : _functions)
        if (f.getName() == name) {
            _functions.remove(&f);
            delete &f;
            return;
        }
};

} // end namespace glu::gil
