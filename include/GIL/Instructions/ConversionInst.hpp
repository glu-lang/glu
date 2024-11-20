#ifndef GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {
/// @class ConversionInst
/// @brief A class representing a conversion instruction in the GIL.
///
/// This class inherits from InstBase and provides functionality specific to
/// conversion instructions, which are instructions with exactly two operands
/// (one type, one value) and one result.
class ConversionInst : public InstBase {
protected:
    Type destType;
    Value operand;

public:
    Type getDestType() const { return destType; }
    Value getOperand() const { return operand; }

    size_t getResultCount() const override { return 1; }
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return destType;
    }
    size_t getOperandCount() const override { return 2; }
    Operand getOperand(size_t index) const override
    {
        switch (index) {
        case 0: return getDestType();
        case 1: return getOperand();
        default: llvm_unreachable("Invalid operand index");
        }
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::ConversionInstFirstKind
            && inst->getKind() <= InstKind::ConversionInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP
