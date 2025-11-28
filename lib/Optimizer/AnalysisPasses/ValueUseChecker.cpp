#include "Optimizer/AnalysisPasses.hpp"

namespace glu::optimizer {

using namespace glu::gil;

/// @brief Helper that verifies whether a value is only used by a specific
/// instruction. Needed because GIL values do not expose direct use iterators.
class ValueUseChecker : public InstVisitor<ValueUseChecker> {
private:
    Value target;
    InstBase *allowedUser;
    std::size_t _useCount = 0;
    bool _onlyAllowedUser = true;

    void recordUse(InstBase *inst)
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
        if constexpr (std::is_same_v<OperandType, Value>) {
            if ((inst->*getter)() == target) {
                recordUse(inst);
            }
        } else if constexpr (std::is_same_v<
                                 OperandType,
                                 std::variant<Value, Function *>>) {
            auto operand = (inst->*getter)();
            if (std::holds_alternative<Value>(operand)
                && std::get<Value>(operand) == target) {
                recordUse(inst);
            }
        }
    }

    template <typename ConcreteInstType, typename RangeType>
    void inspectOperandList(ConcreteInstType *inst, RangeType range)
    {
        using ElemTy = typename std::remove_reference_t<RangeType>::value_type;
        auto recordIfMatch = [&](Value candidate) {
            if (candidate == target) {
                recordUse(inst);
            }
        };

        if constexpr (std::is_same_v<ElemTy, Value>) {
            for (auto const &operand : range) {
                recordIfMatch(operand);
            }
        } else if constexpr (std::is_same_v<
                                 ElemTy, std::variant<Value, Function *>>) {
            for (auto const &operand : range) {
                if (std::holds_alternative<Value>(operand)) {
                    recordIfMatch(std::get<Value>(operand));
                }
            }
        }
    }

public:
    ValueUseChecker(Value value, InstBase *user)
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

bool valueIsUsedOnlyBy(Value value, InstBase *user)
{
    auto *function = user->getParent()->getParent();
    ValueUseChecker checker(value, user);
    checker.visit(function);
    return checker.hasOnlyAllowedUse();
}

} // namespace glu::optimizer
