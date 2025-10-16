#include "Basic/Diagnostic.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/BrInst.hpp"
#include "Instructions/CondBrInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "Instructions/UnreachableInst.hpp"
#include "PassManager.hpp"

#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

/// @brief Dead Code Elimination pass.
///
/// This pass removes unreachable basic blocks (blocks which are never branched
/// to). Any unreachable basic block is removed from the function.
/// If the removed instructions have a source location, we generate a warning
/// about the code being unreachable.
class DeadCodeEliminationPass
    : public gil::InstVisitor<DeadCodeEliminationPass> {
private:
    DiagnosticManager &diagManager;
    llvm::DenseSet<gil::BasicBlock *> reachableBlocks;
    llvm::SmallVector<gil::BasicBlock *, 8> blocksToRemove;
    llvm::DenseSet<SourceLocation> warnedLocations;

    /// @brief Compute all reachable basic blocks in a function using DFS.
    void computeReachableBlocks(gil::Function *func)
    {
        reachableBlocks.clear();

        if (func->getBasicBlockCount() == 0)
            return;

        llvm::SmallVector<gil::BasicBlock *, 32> worklist;
        worklist.push_back(func->getEntryBlock());

        while (!worklist.empty()) {
            auto *bb = worklist.pop_back_val();

            // Mark as reachable; skip if already visited
            auto [_, inserted] = reachableBlocks.insert(bb);
            if (!inserted)
                continue;

            auto *terminator = bb->getTerminator();
            if (!terminator)
                continue;

            if (auto *brInst = llvm::dyn_cast<gil::BrInst>(terminator)) {
                if (auto *dest = brInst->getDestination())
                    worklist.push_back(dest);
            } else if (auto *condBr
                       = llvm::dyn_cast<gil::CondBrInst>(terminator)) {
                if (auto *thenBlock = condBr->getThenBlock())
                    worklist.push_back(thenBlock);
                if (auto *elseBlock = condBr->getElseBlock())
                    worklist.push_back(elseBlock);
            }
            // ReturnInst and UnreachableInst have no successors
        }
    }

public:
    DeadCodeEliminationPass(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }

    void beforeVisitFunction(gil::Function *func)
    {
        blocksToRemove.clear();
        warnedLocations.clear();
        computeReachableBlocks(func);
    }

    void beforeVisitBasicBlock(gil::BasicBlock *bb)
    {
        if (reachableBlocks.contains(bb))
            return;

        // Only warn about unreachable blocks that contain actual user code
        // (not just compiler-generated branches to unreachable)
        for (auto &inst : bb->getInstructions()) {
            // Skip branches and unreachable instructions - these are often
            // compiler-generated
            if (llvm::isa<gil::BrInst>(&inst)
                || llvm::isa<gil::UnreachableInst>(&inst)
                || llvm::isa<gil::ReturnInst>(&inst)
                || llvm::isa<gil::DropInst>(&inst)
                || llvm::isa<gil::LoadInst>(&inst)) {
                continue;
            }

            // If we find any other instruction with a valid location,
            // it's user code
            if (inst.getLocation().isValid()) {
                auto [_, inserted] = warnedLocations.insert(inst.getLocation());
                if (inserted) {
                    diagManager.warning(
                        inst.getLocation(), "Code is unreachable"
                    );
                }
                break;
            }
        }

        blocksToRemove.push_back(bb);
    }

    void afterVisitFunction(gil::Function *func)
    {
        // Remove unreachable blocks
        for (auto *bb : blocksToRemove) {
            func->removeBasicBlock(bb);
        }
        blocksToRemove.clear();
        reachableBlocks.clear();
    }
};

void PassManager::runDeadCodeEliminationPass(
    gil::Module *module, llvm::BumpPtrAllocator & /* arena */,
    DiagnosticManager &diagManager
)
{
    DeadCodeEliminationPass pass(diagManager);
    pass.visit(module);
}

} // namespace glu::gilgen
