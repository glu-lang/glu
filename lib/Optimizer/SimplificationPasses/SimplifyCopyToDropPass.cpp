#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "Instructions/Memory/LoadInst.hpp"
#include "Instructions/OSSA/DropInst.hpp"
#include "Optimizer/AnalysisPasses.hpp"
#include "PassManager.hpp"

#include <vector>

namespace glu::optimizer {

/// @class SimplifyCopyToDropPass
/// @brief Simplifies load [copy] + drop patterns into load [take].
///
/// This pass detects patterns where a value is copied and then the original
/// is immediately dropped, which is wasteful. Instead, we can just take
/// ownership (move) of the value directly.
///
/// Since drop now takes a pointer directly, this pass looks for:
///   %1 = load [copy] %0
///   drop %0
///   ... use %1 ...
/// and transforms it into:
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
        // Get the pointer being dropped
        gil::Value address = dropInst->getValue();

        // Look for a load [copy] from the same address that comes before
        gil::LoadInst *loadCopy = nullptr;
        for (auto &inst : dropInst->getParent()->getInstructions()) {
            if (&inst == dropInst) {
                // We've reached the drop, stop looking
                break;
            }
            if (auto *load = llvm::dyn_cast<gil::LoadInst>(&inst)) {
                if (load->getValue() == address
                    && load->getOwnershipKind()
                        == gil::LoadOwnershipKind::Copy) {
                    loadCopy = load;
                    // Keep looking for the last one before drop
                }
            }
        }

        if (!loadCopy) {
            return;
        }

        if (loadCopy->getParent() != dropInst->getParent()) {
            return; // Must be in the same basic block
        }

        // Check that no instruction uses the address between load [copy] and
        // drop except the load [copy] itself
        bool foundCopy = false;
        for (auto &inst : dropInst->getParent()->getInstructions()) {
            if (&inst == loadCopy) {
                foundCopy = true;
                continue;
            }
            if (&inst == dropInst) {
                // We've reached the drop, we're safe
                break;
            }
            if (foundCopy) {
                // Check if this instruction uses the address
                if (instructionUsesValue(&inst, address)) {
                    return; // Unsafe to optimize
                }
            }
        }

        // Transform: change load [copy] to load [take], remove drop
        loadCopy->setOwnershipKind(gil::LoadOwnershipKind::Take);
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
