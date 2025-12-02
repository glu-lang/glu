#include "Context.hpp"
#include "Types.hpp"

#include <llvm/Support/Casting.h>

using namespace glu::gilgen;
using namespace glu::ast;

glu::gilgen::Context::Context(
    gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
)
    : _module(module), _functionDecl(decl), _globalCtx(&globalCtx)
{
    _function = getOrCreateGILFunction(decl);

    llvm::SmallVector<gil::Type, 8> params;
    for (auto *type : decl->getType()->getParameters()) {
        params.emplace_back(type);
    }
    _currentBB = gil::BasicBlock::create("entry", params);

    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gilgen::Context::Context(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
    : _module(module), _functionDecl(nullptr), _globalCtx(&globalCtx)
{
    auto funcName = std::string(decl->getName()) + ".init";
    _function = createNewGILFunction(
        funcName,
        decl->getModule()
            ->getContext()
            ->getTypesMemoryArena()
            .create<types::FunctionTy>(
                llvm::ArrayRef<glu::types::TypeBase *> {}, decl->getType()
            ),
        nullptr
    );

    _currentBB = gil::BasicBlock::create("entry", llvm::ArrayRef<gil::Type> {});
    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gilgen::Context::Context(gil::Module *module, gil::Function *function)
    : _function(function)
    , _currentBB(nullptr)
    , _module(module)
    , _functionDecl(function->getDecl())
{
    // Don't create any new basic blocks, just work with the existing function
    // The insertion point will be set explicitly later
}
