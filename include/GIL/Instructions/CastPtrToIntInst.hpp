#ifndef GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

class CastPtrToIntInst : public InstBase {
    Value _value; ///< The pointer value to be casted.
    Type _type; ///< The target integer type.

public:
    /// @brief Constructs a CastPtrToIntInst object.
    ///
    /// @param value The pointer value to be casted.
    /// @param type The target integer type.
    CastPtrToIntInst(Value value, Type type)
        : InstBase(InstKind::CastPtrToIntInstKind), _value(value), _type(type)
    {
    }

    /// @brief Gets the number of operands.
    ///
    /// @return The number of operands.
    size_t getOperandCount() const override { return 2; }

    /// @brief Gets the operand at the specified index.
    ///
    /// @param index The index of the operand.
    /// @return The operand at the specified index.
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return _value;
        if (index == 1)
            return Operand(_type);
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

    /// @brief Checks if the given instruction is of type CastPtrToIntInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type CastPtrToIntInst, false otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CastPtrToIntInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP
