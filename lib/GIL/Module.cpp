#include "Module.hpp"

namespace glu::gil {

void Module::addFunction(
    std::string name, glu::types::FunctionTy *type,
    std::list<BasicBlock> basicBlocks
)
{
    Function *f = &_functions.emplace_back(name, type, this);

    for (auto &bb : basicBlocks)
        f->addBasicBlockAtEnd(&bb);
};

Function const *Module::getFunction(std::string name)
{
    for (auto &f : _functions)
        if (f.getName() == name)
            return &f;
    return nullptr;
};
}
