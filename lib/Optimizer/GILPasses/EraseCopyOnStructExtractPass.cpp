#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "Instructions/LoadInst.hpp"
#include "Instructions/ReturnInst.hpp"
#include "Instructions/StoreInst.hpp"
#include "Instructions/StructExtractInst.hpp"
#include "Instructions/StructFieldPtrInst.hpp"
#include "PassManager.hpp"
#include <vector>

namespace glu::optimizer {

/// @class EraseCopyOnStructExtractPass
/// @brief An optimization pass that transforms load [copy] + struct_extract
/// patterns into struct_field_ptr + load [copy] patterns to avoid copying the
/// entire struct. This pass transforms patterns like `%1 = load [copy] %0` `%2
/// = struct_extract %1` into `%1 = struct_field_ptr %0` `%2 = load [copy] %1`
/// This avoids copying the entire struct when only one field is needed, while
/// still properly copying the field if it has non-trivial ownership.
class EraseCopyOnStructExtractPass
    : public gil::InstVisitor<EraseCopyOnStructExtractPass> {
private:
    gil::Module *module;
    std::optional<gilgen::Context> ctx = std::nullopt;
    llvm::BumpPtrAllocator &arena;

    /// @brief Instructions to erase at the end
    std::vector<gil::InstBase *> toErase;

public:
    /// @brief Constructor
    EraseCopyOnStructExtractPass(
        gil::Module *module, llvm::BumpPtrAllocator &arena
    )
        : module(module), arena(arena)
    {
    }

    /// @brief Destructor, performs deferred instruction deletions
    ~EraseCopyOnStructExtractPass()
    {
        // Delete all marked instructions
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
    }

    /// @brief Visits a struct_extract instruction and tries to optimize the
    /// pattern.
    ///
    /// @param extractInst The struct_extract instruction to visit
    void visitStructExtractInst(gil::StructExtractInst *extractInst)
    {
        if (!ctx)
            return;

        // Get the struct value being extracted from
        gil::Value structValue = extractInst->getStructValue();

        // Check if the struct value comes from a load [copy] instruction
        auto *definingInst = structValue.getDefiningInstruction();
        if (!definingInst || !llvm::isa<gil::LoadInst>(definingInst)) {
            return;
        }

        auto *loadInst = llvm::cast<gil::LoadInst>(definingInst);

        // Only optimize load [copy] instructions
        if (loadInst->getOwnershipKind() != gil::LoadOwnershipKind::Copy) {
            return;
        }

        // Get the pointer to the struct
        gil::Value structPtr = loadInst->getValue();

        // Create struct_field_ptr instruction
        auto *bb = extractInst->getParent();
        ctx->setInsertionPoint(bb, extractInst);
        ctx->setSourceLoc(extractInst->getLocation());

        auto *fieldPtrInst
            = ctx->buildStructFieldPtr(structPtr, extractInst->getMember());

        // Get the field type from the extract result
        gil::Type fieldType = extractInst->getResultType(0);

        // Create load [copy] from the field pointer
        auto *newLoadInst = ctx->buildLoad(
            fieldType, fieldPtrInst->getResult(0), gil::LoadOwnershipKind::Copy
        );

        // Replace all uses of the extract result with the new load result
        gil::Value oldValue = extractInst->getResult(0);
        gil::Value newValue = newLoadInst->getResult(0);

        auto *func = extractInst->getParent()->getParent();
        // TODO: replaceAllUses(func, oldValue, newValue);

        // Mark the extract for deletion
        toErase.push_back(extractInst);

        // Check if the original load has any remaining users
        bool hasOtherUsers = false;
        for (auto &bb : func->getBasicBlocks()) {
            for (auto &inst : bb.getInstructions()) {
                if (&inst == loadInst || &inst == extractInst)
                    continue;

                for (size_t i = 0; i < inst.getOperandCount(); ++i) {
                    auto operand = inst.getOperand(i);
                    if (operand.getKind() == gil::OperandKind::ValueKind
                        && operand.getValue() == loadInst->getResult(0)) {
                        hasOtherUsers = true;
                        break;
                    }
                }
                if (hasOtherUsers)
                    break;
            }
            if (hasOtherUsers)
                break;
        }

        // If the load has no other users, mark it for deletion
        if (!hasOtherUsers) {
            toErase.push_back(loadInst);
        }
    }

    /// @brief Called before visiting a function
    ///
    /// @param func The function about to be visited
    void beforeVisitFunction(gil::Function *func)
    {
        // Clear state for each function
        toErase.clear();

        // Create context for this function
        ctx.emplace(module, func, arena);
    }

    /// @brief Called after visiting a function
    ///
    /// @param func The function that was just visited
    void afterVisitFunction(gil::Function *) { ctx.reset(); }
};

void PassManager::runEraseCopyOnStructExtractPass()
{
    EraseCopyOnStructExtractPass pass(_module, _gilArena);
    pass.visit(_module);
}

} // namespace glu::optimizer
