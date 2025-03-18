#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_HPP

#include "InstBase.hpp"

namespace glu::gil {

///
/// @class StructDestructureInst
/// @brief Represents an instruction to extract a member from a structure.
///
/// This class is derived from InstBase and represents an instruction to extract
/// a member from a structure in the GLU GIL (Generic Intermediate Language).
/// The first operand is the structure value from which to extract the member,
/// and teh second operand is the member to extract.
///
class StructDestructureInst : public InstBase {
    Value
        structValue; ///< The structure value from which to extract the member.

public:
    ///
    /// @brief Constructs a StructDestructureInst object.
    ///
    /// @param structValue The structure value from which to extract the member.
    /// @param member The member to extract from the structure.
    ///
    StructDestructureInst(Value structValue)
        : InstBase(InstKind::StructDestructureInstKind)
        , structValue(structValue)
    {
    }

    ///
    /// @brief Gets the structure value.
    ///
    /// @return The structure value.
    ///
    Value getStructValue() const { return structValue; }

    virtual size_t getResultCount() const override { return 2; }

    virtual size_t getOperandCount() const override { return 1; }

    Type getStructType(size_t index) const override
    {
        // TODO: return value.getType()
        return Type();
    }

    virtual Operand getOperand(size_t index) const override
    {
        assert(index < getOperandCount() && "Operand index out of range");
        return Operand(structValue);
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructDestructureInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_HPP
