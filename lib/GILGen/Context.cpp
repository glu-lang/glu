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
    _function = getOrCreateGILFunction(decl);

    llvm::SmallVector<gil::Type, 8> params;
    for (auto *type : decl->getType()->getParameters()) {
        params.emplace_back(translateType(type));
    }
    _currentBB = gil::BasicBlock::create(_arena, "entry", params);

    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gilgen::Context::Context(
    gil::Module *module, ast::VarLetDecl *decl, llvm::BumpPtrAllocator &arena
)
    : _module(module), _functionDecl(nullptr), _arena(arena)
{
    auto funcName = std::string(decl->getName()) + ".init";
    auto size = funcName.size();
    auto funcNameStorage = static_cast<char *>(arena.Allocate(size, 1));
    std::memcpy(funcNameStorage, funcName.data(), size);
    _function = createNewGILFunction(
        llvm::StringRef(funcNameStorage, size),
        decl->getModule()
            ->getContext()
            ->getTypesMemoryArena()
            .create<types::FunctionTy>(
                llvm::ArrayRef<glu::types::TypeBase *> {}, decl->getType()
            ),
        nullptr
    );

    _currentBB = gil::BasicBlock::create(
        _arena, "entry", llvm::ArrayRef<gil::Type> {}
    );
    _function->addBasicBlockAtEnd(_currentBB);
}

glu::gil::Type Context::translateType(types::TypeBase *type)
{
    return TypeTranslator().visit(type);
}
