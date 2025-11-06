#include "GIL/Instructions/CopyInst.hpp"
#include "GIL/Instructions/StructExtractInst.hpp"
#include "GILGen/Context.hpp"
#include "PassManager.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::optimizer {

/// @brief Erase Copy On Struct Extract Pass
///
/// This pass removes unnecessary copy instructions when the copy result is only
/// used by a struct extract instruction. Instead of copying the entire struct
/// and then extracting a field, we directly extract from the original struct.
///
/// Example transformation:
/// Before:
///   %1 = copy %0
///   %2 = struct_extract %1, field
///
/// After:
///   %2 = struct_extract %0, field
///
/// This optimization is safe because struct extraction doesn't modify the
/// struct, so we don't need to copy it first just to extract a field.
class EraseCopyOnStructExtractPass
    : public gil::InstVisitor<EraseCopyOnStructExtractPass> {
private:
    llvm::BumpPtrAllocator &arena;

    // Track which values are used by instructions (for dead copy elimination)
    llvm::DenseMap<gil::Value, llvm::SmallVector<gil::InstBase *, 4>>
        valueUsers;

    // Track copies that should be erased
    llvm::DenseSet<gil::InstBase *> toErase;

public:
    EraseCopyOnStructExtractPass(llvm::BumpPtrAllocator &arena) : arena(arena)
    {
    }

    ~EraseCopyOnStructExtractPass()
    {
        // Erase all marked instructions at the end
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
    }

    /// @brief Build a map of which instructions use each value
    void beforeVisitFunction(gil::Function *func)
    {
        valueUsers.clear();

        // First pass: collect all value uses
        for (auto &bb : func->getBasicBlocks()) {
            for (auto &inst : bb.getInstructions()) {
                for (size_t i = 0; i < inst.getOperandCount(); ++i) {
                    auto operand = inst.getOperand(i);
                    if (operand.getKind() == gil::OperandKind::ValueKind) {
                        gil::Value value = operand.getValue();
                        // Skip empty/tombstone values
                        if (value != gil::Value::getEmptyKey()
                            && value != gil::Value::getTombstoneKey()) {
                            valueUsers[value].push_back(&inst);
                        }
                    }
                }
            }
        }
    }

    void visitStructExtractInst(gil::StructExtractInst *extractInst)
    {
        // Get the struct value being extracted from
        gil::Value structValue = extractInst->getStructValue();

        // Check if this value comes from a CopyInst
        auto *definingInst = structValue.getDefiningInstruction();
        if (!definingInst)
            return; // Not defined by an instruction (e.g., function argument)

        auto *copyInst = llvm::dyn_cast<gil::CopyInst>(definingInst);
        if (!copyInst)
            return; // Not a copy instruction

        // Get the original value that was copied
        gil::Value originalValue = copyInst->getSource();

        // Replace the struct extract's operand with the original value
        // (before the copy)
        extractInst->setStructValue(originalValue);

        // Get the copy result value
        gil::Value copyResult = copyInst->getResult(0);

        // Check if the copy is now dead (no other users)
        // Only check if the value is valid (not empty/tombstone)
        if (copyResult != gil::Value::getEmptyKey()
            && copyResult != gil::Value::getTombstoneKey()) {
            auto it = valueUsers.find(copyResult);
            if (it != valueUsers.end()) {
                auto &users = it->second;

                // Remove this extract instruction from the users list
                users.erase(
                    std::remove(users.begin(), users.end(), extractInst),
                    users.end()
                );

                // If no more users, mark the copy for deletion
                if (users.empty()) {
                    toErase.insert(copyInst);
                }
            }
        }
    }
};

void PassManager::runEraseCopyOnStructExtractPass()
{
    EraseCopyOnStructExtractPass pass(_gilArena);
    pass.visit(_module);
}

} // namespace glu::optimizer
