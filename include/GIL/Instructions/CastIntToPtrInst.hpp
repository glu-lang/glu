#ifndef GLU_GIL_CAST_INT_TO_PTR_INST_HPP
#define GLU_GIL_CAST_INT_TO_PTR_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

/// @class CastIntToPtrInst
/// @brief Represents an instruction to cast an integer to a pointer.
///
/// This class is derived from InstBase and represents an instruction
/// to cast an integer to a pointer in the GLU GIL (Generic Intermediate Language).
class CastIntToPtrInst : public InstBase {
    Value _value; ///< The integer value to be casted.
    Type _type; ///< The target pointer type.

public:
    /// @brief Constructs a CastIntToPtrInst object.
    ///
    /// @param value The integer value to be casted.
    /// @param type The target pointer type.
    CastIntToPtrInst(Value value, Type type)
        : InstBase(InstKind::CastIntToPtrInstKind), _value(value), _type(type)
    {
    }

    /// @brief Gets the number of operands.
    ///
    /// @return The number of operands.
    size_t getOperandCount() const override { return 1; }

    /// @brief Gets the operand at the specified index.
    ///
    /// @param index The index of the operand.
    /// @return The operand at the specified index.
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return _value;
        else
            assert(false && "Invalid operand index");
    }

    /// @brief Gets the number of results.
    ///
    /// @return The number of results.
    size_t getResultCount() const override { return 1; }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type.
    /// @return The result type at the specified index.
    Type getResultType(size_t index) const override { return _type; }

    /// @brief Checks if the given instruction is of type CastIntToPtrInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type CastIntToPtrInst, false otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CastIntToPtrInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_CAST_INT_TO_PTR_INST_HPP
