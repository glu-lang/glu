#include "Context.hpp"

using namespace glu::gilgen;
using namespace glu::ast;

glu::gilgen::Context::Context(
    ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
    : _functionDecl(decl), _arena(arena)
{
    _function
        = new (_arena) gil::Function(decl->getName().str(), decl->getType());
    _currentBB = gil::BasicBlock::create(_arena, "entry", {});
    _function->addBasicBlockAtEnd(_currentBB);
}
