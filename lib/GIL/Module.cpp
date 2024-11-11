#include "Module.hpp"

namespace glu::gil {

void Module::addFunction(
    std::string name, glu::types::FunctionTy *type,
    std::list<BasicBlock> basicBlocks
)
{
    Function f(name, type);

    for (auto &bb : basicBlocks)
        f.addBasicBlockAtEnd(&bb);
    _functions.push_back(&f);
};

Function const *Module::getFunction(std::string name)
{
    for (auto &f : _functions)
        if (f.getName() == name)
            return &f;
    return nullptr;
};
} // end namespace glu::gil

namespace llvm {
glu::gil::Module *ilist_traits<glu::gil::Function>::getContainingModule()
{
    size_t Offset = reinterpret_cast<size_t>(
        &((glu::gil::Function *) nullptr
              ->*glu::gil::Function::getSublistAccess(
                  static_cast<glu::gil::BasicBlock *>(nullptr)
              ))
    );
    iplist<glu::gil::Function> *Anchor
        = static_cast<iplist<glu::gil::Function> *>(this);
    return reinterpret_cast<glu::gil::Module *>(
        reinterpret_cast<char *>(Anchor) - Offset
    );
}
} // end namespace llvm
