#ifndef GLU_GILGEN_GILPASSES_DEADCODEELIMINATION_HPP
#define GLU_GILGEN_GILPASSES_DEADCODEELIMINATION_HPP

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
    llvm::SmallVector<SourceLocation, 8> warnedLocations;

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

    /// @brief Check if we've already warned for this source location
    bool alreadyWarned(SourceLocation loc)
    {
        for (auto const &warnedLoc : warnedLocations) {
            if (warnedLoc == loc) {
                return true;
            }
        }
        return false;
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

        // Collect unreachable blocks
        for (auto &bb : func->getBasicBlocks()) {
            if (reachableBlocks.contains(&bb))
                continue;

            // Only warn about unreachable blocks that contain actual user code
            // (not just compiler-generated branches to unreachable)
            for (auto &inst : bb.getInstructions()) {
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
                    if (!alreadyWarned(inst.getLocation())) {
                        diagManager.warning(
                            inst.getLocation(), "unreachable code detected"
                        );
                        warnedLocations.push_back(inst.getLocation());
                    }
                    break;
                }
            }

            blocksToRemove.push_back(&bb);
        }
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

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILPASSES_DEADCODEELIMINATION_HPP
