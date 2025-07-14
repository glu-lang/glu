#include "Context.hpp"
#include "TypeTranslator.hpp"
#include "Types.hpp"

#include <llvm/Support/Casting.h>

using namespace glu::gilgen;
using namespace glu::ast;

glu::gilgen::Context::Context(
    gil::Module *module, ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
    : _module(module), _functionDecl(decl), _arena(arena)
{
    _function = new (_arena) gil::Function(decl->getName(), decl->getType());

    llvm::SmallVector<gil::Type, 8> params;
    for (auto *type : decl->getType()->getParameters()) {
        params.emplace_back(translateType(type));
    }
    _currentBB = gil::BasicBlock::create(_arena, "entry", params);

    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gil::Type Context::translateType(types::TypeBase *type)
{
    return TypeTranslator().visit(type);
}
