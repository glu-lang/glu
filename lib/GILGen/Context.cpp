#include "Context.hpp"

using namespace glu::gilgen;
using namespace glu::ast;

glu::gilgen::Context::Context(
    ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
    : _functionDecl(decl), _arena(arena)
{
    _function = new (_arena) gil::Function(decl->getName(), decl->getType());
    _currentBB = new (_arena)
        gil::BasicBlock("entry"); // TODO: args are the function's arg
    _function->addBasicBlockAtEnd(_currentBB);
}
