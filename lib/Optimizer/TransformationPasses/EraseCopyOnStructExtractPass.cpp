#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "Instructions/Aggregates/StructExtractInst.hpp"
#include "Instructions/Aggregates/StructFieldPtrInst.hpp"
#include "Instructions/Memory/LoadInst.hpp"
#include "Optimizer/AnalysisPasses/ValueUseChecker.hpp"
#include "PassManager.hpp"
#include <type_traits>
#include <variant>
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

    /// @brief Instructions to erase at the end
    std::vector<gil::InstBase *> toErase;

public:
    /// @brief Constructor
    EraseCopyOnStructExtractPass(gil::Module *module) : module(module) { }

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

        gil::Value loadValue = loadInst->getResult(0);
        bool loadUsedOnlyByExtract = valueIsUsedOnlyBy(loadValue, extractInst);

        auto *fieldPtrInst
            = ctx->buildStructFieldPtr(structPtr, extractInst->getMember());

        // Get the field type from the extract result
        gil::Type fieldType = extractInst->getResultType();

        // Create load [copy] from the field pointer and reuse it as the
        // replacement value for the extract.
        auto *fieldLoadInst = ctx->buildLoad(
            fieldType, fieldPtrInst->getResult(0), gil::LoadOwnershipKind::Copy
        );

        extractInst->getResult(0).replaceAllUsesWith(
            fieldLoadInst->getResult(0)
        );

        // Mark original instructions for deletion once visitation completes.
        toErase.push_back(extractInst);
        if (loadUsedOnlyByExtract) {
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
        ctx.emplace(module, func);
    }

    /// @brief Called after visiting a function
    ///
    /// @param func The function that was just visited
    void afterVisitFunction(gil::Function *)
    {
        for (auto *inst : toErase) {
            inst->eraseFromParent();
        }
        toErase.clear();
        ctx.reset();
    }
};

void PassManager::runEraseCopyOnStructExtractPass()
{
    EraseCopyOnStructExtractPass pass(_module);
    pass.visit(_module);
}

} // namespace glu::optimizer
