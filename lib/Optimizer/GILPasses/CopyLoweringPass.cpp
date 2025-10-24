#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "Instructions/LoadInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

class CopyLoweringPass : public gil::InstVisitor<CopyLoweringPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;
    llvm::BumpPtrAllocator &arena;
    llvm::SmallVector<gil::InstBase *, 8> toErase;
    bool inCopyFunction = false; // Track if we're inside a copy function

public:
    CopyLoweringPass(gil::Module *module, llvm::BumpPtrAllocator &arena)
        : module(module), arena(arena)
    {
    }

    ~CopyLoweringPass()
    {
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
    }

    void visitLoadInst(gil::LoadInst *loadInst)
    {
        // Don't transform loads inside copy functions (would cause infinite
        // recursion)
        if (inCopyFunction)
            return;

        // Only handle load [copy] instructions
        if (loadInst->getOwnershipKind() != gil::LoadOwnershipKind::Copy)
            return;

        if (!ctx)
            return;

        // Generate code to call the copy function if it exists
        auto *structure = llvm::dyn_cast<types::StructTy>(
            loadInst->getResultType(0).getType()
        );
        if (structure && structure->getDecl()->hasOverloadedCopyFunction()) {
            // Change the load to trivial ownership (no copy semantics)
            loadInst->setOwnershipKind(gil::LoadOwnershipKind::Trivial);

            // Insert a call to the copy function after the load
            auto *bb = loadInst->getParent();
            auto it = std::next(loadInst->getIterator());
            gil::InstBase *nextInst
                = (it != bb->getInstructions().end()) ? &*it : nullptr;

            ctx->setInsertionPoint(bb, nextInst);
            ctx->setSourceLoc(loadInst->getLocation());

            // Call the copy function with the loaded value
            auto *callInst = ctx->buildCall(
                structure->getDecl()->getCopyFunction(),
                { loadInst->getResult(0) }
            );

            // Find the store that uses this load's result and update it
            if (nextInst && llvm::isa<gil::StoreInst>(nextInst)) {
                auto *storeInst = llvm::cast<gil::StoreInst>(nextInst);
                if (storeInst->getSource() == loadInst->getResult(0)) {
                    // Replace the store's source with the call result
                    storeInst->setSource(callInst->getResult(0));
                }
            }
        }
    }

    void visitCopyInst(gil::CopyInst *copyInst)
    {
        if (!ctx)
            return;

        auto *bb = copyInst->getParent();
        ctx->setInsertionPoint(bb, copyInst);
        ctx->setSourceLoc(copyInst->getLocation());

        // Generate code to call the copy function if it exists
        if (auto *structure = llvm::dyn_cast<types::StructTy>(
                copyInst->getSource().getType().getType()
            )) {
            if (structure->getDecl()->hasOverloadedCopyFunction()) {
                auto *callInst = ctx->buildCall(
                    structure->getDecl()->getCopyFunction(),
                    { copyInst->getSource() }
                );
                // Replace the copy instruction with the call instruction
                bb->replaceInstruction(copyInst, callInst);
                return; // Don't erase, we replaced it
            }
        }

        // Remove the original copy instruction if no custom copy function
        toErase.push_back(copyInst);
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Check if this is a copy function
        if (func->getName() == "copy") {
            inCopyFunction = true;
        }

        // Create context for this function
        ctx.emplace(module, func, arena);
    }

    void afterVisitFunction(gil::Function *)
    {
        ctx.reset();
        inCopyFunction = false;
    }
};

void PassManager::runCopyLoweringPass()
{
    CopyLoweringPass pass(_module, _gilArena);
    pass.visit(_module);
}

} // namespace glu::optimizer
