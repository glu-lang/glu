#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP

#include "AggregateInst.hpp"
#include "Types.hpp"

#include <vector>

namespace glu::gil {

/// @class StructCreateInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from AggregateInst and represents an instruction
/// to create a structure literal in the GLU GIL (Generic Intermediate
/// Language).
class StructCreateInst : public AggregateInst {

    Type _structType; ///< The value of the structure.
    llvm::ArrayRef<Type>
        _members; ///< The values for each member of the structure.

public:
    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param _structType The type of the structure.
    /// @param _members The values for each member of the structure.
    StructCreateInst(Type structType, llvm::ArrayRef<Type> operands)
        : AggregateInst(InstKind::StructCreateInstKind)
        , _structType(structType)
        , _members(std::move(operands))
    {
        assert(
            structType.getType()->getKind()
                == glu::types::TypeKind::StructTyKind
            && "Invalid structure type"
        );
    }

    /// @brief Sets the structure value.
    ///
    /// @param value The new structure value.
    void setStruct(Type value) { this->_structType = value; }

    /// @brief Gets the structure value.
    ///
    /// @return The structure value.
    Type getStruct() const { return _structType; }

    /// @brief Sets the values for the members of the structure.
    ///
    /// @param members The new values for the structure members.
    void setMembersTypes(llvm::ArrayRef<Type> members)
    {
        this->_members = std::move(members);
    }

    /// @brief Gets the values of the structure members.
    ///
    /// @return A vector containing the values of all structure members.
    llvm::ArrayRef<Type> getMembersTypes() const { return _members; }

    /// @brief Gets the number of operands required by this instruction.
    ///
    /// @return 1 (for the structure) plus the number of member values.
    size_t getOperandCount() const override { return 1 + _members.size(); }

    /// @brief Gets the operand at the specified index.
    ///
    /// @param index The index of the operand (0 for structure, 1+ for members).
    /// @return The operand at the specified index.
    Operand getOperand(size_t index) const override
    {
        assert(index < getOperandCount() && "Operand index out of range");
        if (index == 0)
            return Operand(_structType);
        return Operand(_members[index - 1]);
    }

    /// @brief Gets the number of results produced by this instruction.
    ///
    /// @return Always 1 - the created structure.
    size_t getResultCount() const override { return 1; }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type (must be 0).
    /// @return The type of the created structure.
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _structType;
    }

    /// @brief Checks if the given instruction is of type StructCreateInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type StructCreateInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructCreateInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
