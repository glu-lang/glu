#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "Instructions/LoadInst.hpp"
#include "Instructions/StoreInst.hpp"
#include "PassManager.hpp"

namespace glu::optimizer {

/// @brief Basic CopyLoweringPass that replaces load [copy] instructions with
/// copy function calls. The copy function receives a pointer to the value.
/// @note This is a simplified version that only handles load instructions.
/// It will generate infinite loops if the copy function itself contains
/// load [copy] instructions, but serves as a foundation for future PRs.
class CopyLoweringPass : public gil::InstVisitor<CopyLoweringPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;

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
        auto *structure = llvm::dyn_cast<types::StructTy>(
            loadInst->getResultType().getType()
        );
        if (!structure || !structure->getDecl()->hasOverloadedCopyFunction())
            return;

        // Change the load to None ownership (no copy semantics)
        loadInst->setOwnershipKind(gil::LoadOwnershipKind::None);

        // Insert a call to the copy function after the load
        auto *bb = loadInst->getParent();
        auto it = std::next(loadInst->getIterator());
        gil::InstBase *nextInst
            = (it != bb->getInstructions().end()) ? &*it : nullptr;

        ctx->setInsertionPoint(bb, nextInst);
        ctx->setSourceLoc(loadInst->getLocation());

        // Create a temporary alloca to hold the loaded value
        auto *tempAlloca = ctx->buildAlloca(loadInst->getResultType());

        // Store the loaded value into the temporary
        ctx->buildStore(loadInst->getResult(0), tempAlloca->getResult(0));

        // Call the copy function with a pointer to the loaded value
        auto *callInst = ctx->buildCall(
            structure->getDecl()->getCopyFunction(),
            { tempAlloca->getResult(0) }
        );

        // Update the next instruction if it uses this load's result
        // This is a simplified approach - just handle the immediate store case
        if (nextInst && llvm::isa<gil::StoreInst>(nextInst)) {
            auto *storeInst = llvm::cast<gil::StoreInst>(nextInst);
            if (storeInst->getSource() == loadInst->getResult(0)) {
                // TODO: Replace with inst->replaceAllUsesWith() when
                // implemented
                storeInst->setSource(callInst->getResult(0));
            }
        }
    }

    void beforeVisitFunction(gil::Function *func)
    {
        // Create context for this function
        ctx.emplace(module, func);
    }

    void afterVisitFunction(gil::Function *) { ctx.reset(); }
};

void PassManager::runCopyLoweringPass()
{
    CopyLoweringPass pass(_module);
    pass.visit(_module);
}

} // namespace glu::optimizer
