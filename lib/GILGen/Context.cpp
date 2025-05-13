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
    _function = new (_arena) gil::Function(decl->getName(), decl->getType());

    _currentBB = gil::BasicBlock::create(
        _arena, "entry", decl->getType()->getParameters()
    );

    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gil::Type Context::translateType(types::TypeBase *type)
{
    return TypeTranslator().visit(type);
}
