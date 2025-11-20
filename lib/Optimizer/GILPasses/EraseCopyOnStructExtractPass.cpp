#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"
#include "GILGen/Context.hpp"
#include "Instructions/Aggregates/StructExtractInst.hpp"
#include "Instructions/Aggregates/StructFieldPtrInst.hpp"
#include "Instructions/Memory/LoadInst.hpp"
#include "PassManager.hpp"
#include <type_traits>
#include <variant>
#include <vector>

namespace glu::optimizer {

namespace {

using namespace glu::gil;

/// @brief Helper that verifies whether a value is only used by a specific
/// instruction. Needed because GIL values do not expose direct use iterators.
class ValueUseChecker : public gil::InstVisitor<ValueUseChecker> {
private:
    gil::Value target;
    gil::InstBase *allowedUser;
    std::size_t _useCount = 0;
    bool _onlyAllowedUser = true;

    void recordUse(gil::InstBase *inst)
    {
        ++_useCount;
        if (inst != allowedUser) {
            _onlyAllowedUser = false;
        }
    }

    template <
        typename ConcreteInstType, typename AccessorInstType,
        typename OperandType>
    void inspectOperand(
        ConcreteInstType *inst, OperandType (AccessorInstType::*getter)() const
    )
    {
        if constexpr (std::is_same_v<OperandType, gil::Value>) {
            if ((inst->*getter)() == target) {
                recordUse(inst);
            }
        } else if constexpr (std::is_same_v<
                                 OperandType,
                                 std::variant<gil::Value, gil::Function *>>) {
            auto operand = (inst->*getter)();
            if (std::holds_alternative<gil::Value>(operand)
                && std::get<gil::Value>(operand) == target) {
                recordUse(inst);
            }
        }
    }

    template <typename ConcreteInstType, typename RangeType>
    void inspectOperandList(ConcreteInstType *inst, RangeType range)
    {
        using ElemTy = typename std::remove_reference_t<RangeType>::value_type;
        if constexpr (std::is_same_v<ElemTy, gil::Value>) {
            for (auto const &operand : range) {
                if (operand == target) {
                    recordUse(inst);
                }
            }
        }
    }

public:
    ValueUseChecker(gil::Value value, gil::InstBase *user)
        : target(value), allowedUser(user)
    {
    }

#define GIL_OPERAND(Name) inspectOperand(inst, &LocalInstType::get##Name)
#define GIL_OPERAND_LIST(Name) inspectOperandList(inst, inst->get##Name())
#define GIL_INSTRUCTION_(CLS, Name, Super, Result, ...) \
    void visit##CLS([[maybe_unused]] CLS *inst)         \
    {                                                   \
        using LocalInstType [[maybe_unused]] = CLS;     \
        __VA_ARGS__;                                    \
    }
#include "GIL/InstKind.def"
#undef GIL_OPERAND
#undef GIL_OPERAND_LIST
#undef GIL_INSTRUCTION_

    bool hasOnlyAllowedUse() const
    {
        return _useCount == 1 && _onlyAllowedUser;
    }
};

bool valueUsedOnlyBy(gil::Value value, gil::InstBase *user)
{
    auto *function = user->getParent()->getParent();
    ValueUseChecker checker(value, user);
    checker.visit(function);
    return checker.hasOnlyAllowedUse();
}

} // namespace

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
        bool loadUsedOnlyByExtract = valueUsedOnlyBy(loadValue, extractInst);

        auto *fieldPtrInst
            = ctx->buildStructFieldPtr(structPtr, extractInst->getMember());

        // Get the field type from the extract result
        gil::Type fieldType = extractInst->getResultType();

        // Create load [copy] from the field pointer and reuse it as the
        // replacement value for the extract.
        auto *fieldLoadInst = ctx->buildLoad(
            fieldType, fieldPtrInst->getResult(0), gil::LoadOwnershipKind::Copy
        );

        extractInst->getResult(0).replaceAllUsesWith(fieldLoadInst->getResult(0)
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
