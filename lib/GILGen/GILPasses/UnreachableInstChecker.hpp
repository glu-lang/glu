#ifndef GLU_GILGEN_GILPASSES_UNREACHABLEINSTCHECKER_HPP
#define GLU_GILGEN_GILPASSES_UNREACHABLEINSTCHECKER_HPP

#include "Basic/Diagnostic.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/BrInst.hpp"
#include "Instructions/CondBrInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "Instructions/UnreachableInst.hpp"

#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

/// @brief GIL Pass that detects reachable unreachable instructions.
///
/// This pass checks for UnreachableInst in reachable basic blocks.
/// Since DCE is not yet implemented, we manually compute reachable blocks
/// by doing a CFG traversal from the entry block.
///
/// This is more accurate than checking the last statement, as it correctly
/// handles if-else branches and other control flow patterns.
class UnreachableInstChecker : public gil::InstVisitor<UnreachableInstChecker> {
private:
    DiagnosticManager &diagManager;
    llvm::DenseSet<gil::BasicBlock *> reachableBlocks;

    /// @brief Compute all reachable basic blocks in a function using DFS.
    void computeReachableBlocks(gil::Function *func)
    {
        reachableBlocks.clear();

        if (func->getBasicBlockCount() == 0)
            return;

        llvm::SmallVector<gil::BasicBlock *, 32> worklist;
        worklist.push_back(func->getEntryBlock());
        reachableBlocks.insert(func->getEntryBlock());

        while (!worklist.empty()) {
            auto *bb = worklist.pop_back_val();
            auto *terminator = bb->getTerminator();
            if (!terminator)
                continue;

            // Add successor blocks to worklist
            if (auto *brInst = llvm::dyn_cast<gil::BrInst>(terminator)) {
                auto *dest = brInst->getDestination();
                if (dest && !reachableBlocks.contains(dest)) {
                    reachableBlocks.insert(dest);
                    worklist.push_back(dest);
                }
            } else if (auto *condBr
                       = llvm::dyn_cast<gil::CondBrInst>(terminator)) {
                auto *thenBlock = condBr->getThenBlock();
                auto *elseBlock = condBr->getElseBlock();
                if (thenBlock && !reachableBlocks.contains(thenBlock)) {
                    reachableBlocks.insert(thenBlock);
                    worklist.push_back(thenBlock);
                }
                if (elseBlock && !reachableBlocks.contains(elseBlock)) {
                    reachableBlocks.insert(elseBlock);
                    worklist.push_back(elseBlock);
                }
            }
            // ReturnInst and UnreachableInst have no successors
        }
    }

public:
    UnreachableInstChecker(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }

    void beforeVisitFunction(gil::Function *func)
    {
        computeReachableBlocks(func);
    }

    void visitUnreachableInst(gil::UnreachableInst *inst)
    {
        auto *bb = inst->getParent();
        if (!bb)
            return;

        // Only report warning if the block containing unreachable is reachable
        if (!reachableBlocks.contains(bb))
            return;

        auto *func = bb->getParent();
        if (!func)
            return;

        auto *decl = func->getDecl();
        if (!decl)
            return;

        // Use the function declaration location for the warning
        SourceLocation loc = decl->getLocation();

        diagManager.warning(
            loc,
            llvm::Twine("Function '") + decl->getName().str()
                + "' does not end with a return statement"
        );
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILPASSES_UNREACHABLEINSTCHECKER_HPP
