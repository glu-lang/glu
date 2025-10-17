#include "../../GILGen/Context.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/ReturnInst.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

class VoidMainPass : public gil::InstVisitor<VoidMainPass> {
private:
    gil::Module *module;
    llvm::BumpPtrAllocator &arena;
    std::optional<gilgen::Context> ctx = std::nullopt;

public:
    VoidMainPass(gil::Module *module, llvm::BumpPtrAllocator &arena)
        : module(module), arena(arena)
    {
    }

    void beforeVisitFunction(gil::Function *func)
    {
        if (func->getName() != "main")
            return;
        if (!llvm::isa<types::VoidTy>(func->getType()->getReturnType()))
            return;

        // Transform the function type from void to int32
        auto *astCtx = func->getDecl()->getModule()->getContext();
        auto &astTyMemArena = astCtx->getTypesMemoryArena();

        auto *newRetType = astTyMemArena.create<types::IntTy>(
            types::IntTy(types::IntTy::Signed, 32)
        );
        auto *newFuncType = astTyMemArena.create<types::FunctionTy>(
            func->getType()->getParameters(), newRetType,
            func->getDecl()->getType()->isCVariadic()
        );
        func->setType(newFuncType);

        ctx.emplace(module, func, arena);
    }

    void afterVisitFunction(gil::Function *) { ctx.reset(); }

    void visitReturnInst(gil::ReturnInst *retInst)
    {
        if (!ctx)
            return;

        // Transform void returns to return 0
        if (retInst->getValue() == gil::Value::getEmptyKey()) {
            auto *bb = retInst->getParent();
            ctx->setInsertionPoint(bb, retInst);

            auto *newRetType
                = ctx->getCurrentFunction()->getType()->getReturnType();
            auto int32Type = ctx->translateType(newRetType);
            auto *zeroValue
                = ctx->buildIntegerLiteral(int32Type, llvm::APInt(32, 0, true));
            retInst->setValue(zeroValue->getResult(0));
        }
    }
};

void PassManager::runVoidMainPass(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager & /* diagManager */
)
{
    VoidMainPass pass(module, arena);
    pass.visit(module);
}

} // namespace glu::optimizer
