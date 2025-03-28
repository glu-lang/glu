#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP

#include "AggregateInst.hpp"

#include <vector>

namespace glu::gil {

/// @class StructCreateInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from AggregateInst and represents an instruction
/// to create a structure literal in the GLU GIL (Generic Intermediate
/// Language).
class StructCreateInst : public AggregateInst {

    Value _struct; ///< The value of the structure.
    std::vector<Value>
        _members; ///< The values for each member of the structure.

public:
    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param _struct The type of the structure.
    /// @param _members The values for each member of the structure.
    StructCreateInst(Value structValue, std::vector<Value> operands)
        : AggregateInst(InstKind::StructCreateInstKind)
        , _struct(structValue)
        , _members(std::move(operands))
    {
    }

    /// @brief Sets the structure value.
    ///
    /// @param value The new structure value.
    void setStruct(Value value) { this->_struct = value; }

    /// @brief Gets the structure value.
    ///
    /// @return The structure value.
    Value getStruct() const { return _struct; }

    /// @brief Sets the values for the members of the structure.
    ///
    /// @param members The new values for the structure members.
    void setMembersValues(std::vector<Value> members)
    {
        this->_members = std::move(members);
    }

    /// @brief Gets the values of the structure members.
    ///
    /// @return A vector containing the values of all structure members.
    std::vector<Value> getMembersValues() const { return _members; }

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
            return Operand(_struct);
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
        return _struct.getType();
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
