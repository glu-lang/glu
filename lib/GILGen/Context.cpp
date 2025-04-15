#include "Context.hpp"
#include "TypeTranslator.hpp"
#include "Types.hpp"

#include <llvm/Support/Casting.h>

using namespace glu::gilgen;
using namespace glu::ast;

glu::gilgen::Context::Context(
    ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
    : _functionDecl(decl), _arena(arena)
{
    _function
        = new (_arena) gil::Function(decl->getName().str(), decl->getType());
    _currentBB = new (_arena)
        gil::BasicBlock("entry"); // TODO: args are the function's arg
    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gil::Type Context::translateType(types::TypeBase *type)
{
    return TypeTranslator().visit(type);
}
