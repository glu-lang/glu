#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

/// @brief Basic CopyLoweringPass that replaces load [copy] instructions with
/// copy function calls. The copy function receives a pointer to the value.
/// @note This pass skips lowering load [copy] instructions when the load
/// is for the same struct type as the copy function we're inside, to avoid
/// infinite recursion. Loading other struct types will still call their
/// respective copy functions.
class CopyLoweringPass : public gil::InstVisitor<CopyLoweringPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;
    std::vector<gil::InstBase *> _instructionsToRemove;

public:
    CopyLoweringPass(gil::Module *module) : module(module) { }

    void visitLoadInst(gil::LoadInst *loadInst)
    {
        // Only handle load [copy] instructions
        if (loadInst->getOwnershipKind() != gil::LoadOwnershipKind::Copy)
            return;

        if (!ctx)
            return;

        // Check if this is a struct type with an overloaded copy function
        auto *structure
            = llvm::dyn_cast<types::StructTy>(loadInst->getResultType());
        if (!structure || !structure->getDecl()->hasOverloadedCopyFunction()) {
            // Change the load to None ownership (no copy semantics)
            loadInst->setOwnershipKind(gil::LoadOwnershipKind::None);
            return;
        }

        ctx->setInsertionPoint(loadInst->getParent(), loadInst);
        ctx->setSourceLoc(loadInst->getLocation());

        // Call the copy function with the original pointer
        auto *callInst = ctx->buildCall(
            structure->getDecl()->getCopyFunction(), { loadInst->getValue() }
        );

        loadInst->getResult(0).replaceAllUsesWith(callInst->getResult(0));
        // Mark the load instruction for removal
        _instructionsToRemove.push_back(loadInst);
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Create context for this function
        ctx.emplace(module, func);
    }

    void afterVisitFunction(gil::Function *)
    {
        ctx.reset();
        for (auto *inst : _instructionsToRemove) {
            inst->eraseFromParent();
        }
        _instructionsToRemove.clear();
    }
};

void PassManager::runCopyLoweringPass()
{
    CopyLoweringPass pass(_module);
    pass.visit(_module);
}

} // namespace glu::optimizer
