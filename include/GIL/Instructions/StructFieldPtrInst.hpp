#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP

#include "AggregateInst.hpp"

namespace glu::gil {

/// @class StructFieldPtrInst
/// @brief Represents an instruction that creates a pointer to a field within a
/// struct.
///
/// This class is derived from InstBase and represents an instruction that
/// calculates and returns a pointer to a specific field of a structure in the
/// GLU GIL (Generic Intermediate Language).
class StructFieldPtrInst : public AggregateInst {
    GLU_GIL_GEN_OPERAND(StructPtr, Value, _structPtr)
    GLU_GIL_GEN_OPERAND(Member, Member, _member)
    Type _ptr; ///< The pointer type to the field

public:
    /// @brief Constructs a StructFieldPtrInst object.
    ///
    /// Creates an instruction that computes a pointer to a field within a
    /// struct.
    ///
    /// @param structPtr The value of the struct being accessed
    /// @param member The descriptor of the field being accessed
    /// @param pointerType The GIL type of the resulting pointer
    StructFieldPtrInst(Value structPtr, Member member, Type pointerType)
        : AggregateInst(InstKind::StructFieldPtrInstKind)
        , _structPtr(structPtr)
        , _member(member)
        , _ptr(pointerType)
    {
    }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type (must be 0).
    /// @return The result type at the specified index (pointer to the field
    /// type).
    Type getResultType() const { return _ptr; }

    /// @brief Checks if the given instruction is of type
    /// StructFieldPtrInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type StructFieldPtrInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructFieldPtrInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP
