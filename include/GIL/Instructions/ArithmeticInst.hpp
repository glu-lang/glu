#ifndef GLU_GIL_INSTRUCTIONS_ARITHMETIC_INST_HPP
#define GLU_GIL_INSTRUCTIONS_ARITHMETIC_INST_HPP

#include "InstBase.hpp"
#include "Type.hpp"
#include "Value.hpp"

namespace glu::gil {

/// @brief Base class for arithmetic instructions
class ArithmeticInst : public InstBase {
private:
    Value _lhs;
    Value _rhs;
    Type _resultType;

protected:
    ArithmeticInst(InstKind kind, Value lhs, Value rhs, Type resultType)
        : InstBase(kind), _lhs(lhs), _rhs(rhs), _resultType(resultType)
    {
    }

public:
    /// @brief Get the left-hand operand
    Value getLHS() const { return _lhs; }

    /// @brief Get the right-hand operand
    Value getRHS() const { return _rhs; }

    // InstBase interface
    size_t getOperandCount() const override { return 2; }

    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return _lhs;
        case 1: return _rhs;
        default: llvm_unreachable("Invalid operand index");
        }
    }

    size_t getResultCount() const override { return 1; }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _resultType;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::AddInstKind
            && inst->getKind() <= InstKind::FRemInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ARITHMETIC_INST_HPP
