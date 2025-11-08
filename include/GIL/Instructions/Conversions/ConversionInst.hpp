#ifndef GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP

#include "../InstBase.hpp"

namespace glu::gil {
/// @class ConversionInst
/// @brief A class representing a conversion instruction in the GIL.
///
/// This class inherits from InstBase and provides functionality specific to
/// conversion instructions, which are instructions with exactly two operands
/// (one type, one value) and one result.
class ConversionInst : public InstBase {
protected:
    GLU_GIL_GEN_OPERAND(DestType, Type, _destType)
    GLU_GIL_GEN_OPERAND(Operand, Value, _operand)

protected:
    /// @brief Constructor for the ConversionInst class.
    /// @param kind The kind of the instruction.
    /// @param destType The destination type of the conversion.
    /// @param operand The operand of the conversion.
    ConversionInst(InstKind kind, Type destType, Value operand)
        : InstBase(kind), _destType(destType), _operand(operand)
    {
    }

public:
    size_t getResultCount() const override { return 1; }
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _destType;
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::ConversionInstFirstKind
            && inst->getKind() <= InstKind::ConversionInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CONVERSION_INST_HPP
