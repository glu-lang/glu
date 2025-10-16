#include "../Context.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/ReturnInst.hpp"
#include "PassManager.hpp"

namespace glu::gilgen {

class DropLoweringPass : public gil::InstVisitor<DropLoweringPass> {
private:
    gil::Module *module;
    std::optional<Context> ctx = std::nullopt;
    llvm::BumpPtrAllocator &arena;
    llvm::SmallVector<gil::InstBase *, 8> toErase;

public:
    DropLoweringPass(gil::Module *module, llvm::BumpPtrAllocator &arena)
        : module(module), arena(arena)
    {
    }

    ~DropLoweringPass()
    {
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
    }

    void visitDropInst(gil::DropInst *dropInst)
    {
        if (!ctx)
            return;

        auto *bb = dropInst->getParent();
        ctx->setInsertionPoint(bb, dropInst);
        ctx->setSourceLoc(dropInst->getLocation());

        // Generate code to call the drop function if it exists
        if (auto *structure = llvm::dyn_cast<types::StructTy>(
                dropInst->getSource().getType().getType()
            )) {
            if (structure->getDecl()->hasOverloadedDropFunction()) {
                ctx->buildCall(
                    structure->getDecl()->getDropFunction(),
                    { dropInst->getSource() }
                );
            }
        }

        // Remove the original drop instruction
        toErase.push_back(dropInst);
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Create context for this function
        ctx.emplace(module, func, arena);
    }

    void afterVisitFunction(gil::Function *) { ctx.reset(); }
};

void PassManager::runDropLoweringPass(
    gil::Module *module, llvm::BumpPtrAllocator &arena,
    DiagnosticManager &diagManager
)
{
    DropLoweringPass pass(module, arena);
    pass.visit(module);
}

} // namespace glu::gilgen
