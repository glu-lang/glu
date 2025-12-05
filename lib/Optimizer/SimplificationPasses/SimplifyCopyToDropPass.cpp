#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/Memory/LoadInst.hpp"
#include "Instructions/OSSA/DropInst.hpp"
#include "Optimizer/AnalysisPasses.hpp"
#include "PassManager.hpp"

#include <vector>

namespace glu::optimizer {

/// @class SimplifyCopyToDropPass
/// @brief Simplifies load [copy] + load [take] + drop patterns into load
/// [take].
///
/// This pass detects patterns where a value is copied and then the original
/// is immediately dropped, which is wasteful. Instead, we can just take
/// ownership (move) of the value directly.
///
/// This pass transforms patterns like:
///   %1 = load [copy] %0
///   %2 = load [take] %0
///   drop %2
///   ... use %1 ...
/// into:
///   %1 = load [take] %0
///   ... use %1 ...
///
/// This avoids unnecessary copies when the original is dropped right after,
/// effectively turning the copy into a move.
class SimplifyCopyToDropPass : public gil::InstVisitor<SimplifyCopyToDropPass> {
private:
    /// @brief Instructions to erase at the end
    std::vector<gil::InstBase *> _toErase;

public:
    /// @brief Constructor
    SimplifyCopyToDropPass() = default;

    /// @brief Visits a drop instruction and checks if it can be optimized.
    ///
    /// @param dropInst The drop instruction to visit
    void visitDropInst(gil::DropInst *dropInst)
    {
        // Get the value being dropped
        gil::Value droppedValue = dropInst->getValue();

        // Check if the dropped value comes from a load [take] instruction
        auto *takeLoadInst = droppedValue.getDefiningInstruction();
        if (!takeLoadInst || !llvm::isa<gil::LoadInst>(takeLoadInst)) {
            return;
        }

        auto *loadTake = llvm::cast<gil::LoadInst>(takeLoadInst);

        // Must be load [take] that's only used by this drop
        if (loadTake->getOwnershipKind() != gil::LoadOwnershipKind::Take) {
            return;
        }
        if (!valueIsUsedOnlyBy(droppedValue, dropInst)) {
            return;
        }

        // Get the address being loaded from
        gil::Value address = loadTake->getValue();

        // Look for a load [copy] from the same address that comes before
        gil::LoadInst *loadCopy = nullptr;
        for (auto &inst : dropInst->getParent()->getInstructions()) {
            if (&inst == takeLoadInst) {
                // We've reached the load [take], stop looking
                break;
            }
            if (auto *load = llvm::dyn_cast<gil::LoadInst>(&inst)) {
                if (load->getValue() == address
                    && load->getOwnershipKind()
                        == gil::LoadOwnershipKind::Copy) {
                    loadCopy = load;
                    // Keep looking for the latest one before load [take]
                }
            }
        }

        if (!loadCopy) {
            return;
        }

        // Transform: change load [copy] to load [take], remove load [take] +
        // drop
        loadCopy->setOwnershipKind(gil::LoadOwnershipKind::Take);
        _toErase.push_back(takeLoadInst);
        _toErase.push_back(dropInst);
    }

    /// @brief Called before visiting a function
    void beforeVisitFunction(gil::Function *) { _toErase.clear(); }

    /// @brief Called after visiting a function
    void afterVisitFunction(gil::Function *)
    {
        for (auto *inst : _toErase) {
            inst->eraseFromParent();
        }
        _toErase.clear();
    }
};

void PassManager::runSimplifyCopyToDropPass()
{
    SimplifyCopyToDropPass pass;
    pass.visit(_module);
}

} // namespace glu::optimizer
