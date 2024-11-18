#include "Module.hpp"

namespace glu::gil {

Function *Module::addFunction(
    std::string name, glu::types::FunctionTy *type,
    std::list<BasicBlock> basicBlocks
)
{
    Function *f = new Function(name, type);

    for (auto &bb : basicBlocks)
        f->addBasicBlockAtEnd(&bb);
    _functions.push_back(std::move(f));
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
