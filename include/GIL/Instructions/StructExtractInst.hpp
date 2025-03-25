#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP

#include "InstBase.hpp"

namespace glu::gil {

///
/// @class StructExtractInst
/// @brief Represents an instruction to extract a member from a structure.
///
/// This class is derived from InstBase and represents an instruction to extract
/// a member from a structure in the GLU GIL (Generic Intermediate Language).
/// The first operand is the structure value from which to extract the member,
/// and teh second operand is the member to extract.
///
class StructExtractInst : public InstBase {
    Value
        structValue; ///< The structure value from which to extract the member.
    Member member; ///< The member to extract from the structure.

public:
    ///
    /// @brief Constructs a StructExtractInst object.
    ///
    /// @param structValue The structure value from which to extract the member.
    /// @param member The member to extract from the structure.
    ///
    StructExtractInst(Value structValue, Member member)
        : InstBase(InstKind::StructExtractInstKind)
        , structValue(structValue)
        , member(member)
    {
    }

    ///
    /// @brief Gets the structure value.
    ///
    /// @return The structure value.
    ///
    Value getStructValue() const { return structValue; }

    ///
    /// @brief Gets the member to extract.
    ///
    /// @return The member to extract.
    ///
    Member getMember() const { return member; }

    virtual size_t getResultCount() const override { return 1; }

    virtual size_t getOperandCount() const override { return 2; }

    /// @brief Returns the type of the result at the specified index.
    /// @param index The index of the result.
    /// @return The type of the result at the specified index.
    virtual Type getResultType(size_t index) const override
    {
        assert(index < getResultCount() && "Result index out of range");
        return member.getValue()->getType();
    }

    virtual Operand getOperand(size_t index) const override
    {
        assert(index < getOperandCount() && "Operand index out of range");
        if (index == 0)
            return Operand(structValue);
        else
            return Operand(member);
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructExtractInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
