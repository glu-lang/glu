#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP

#include "AggregateInst.hpp"
#include "Member.hpp"
#include "Type.hpp"

namespace glu::gil {

/// @class StructDestructureInst
/// @brief Instruction that destructures a struct value into its individual
/// fields.
///
/// It produces N results, where N is the number of fields in the struct.
/// The only operand is the struct value.
class StructDestructureInst : public AggregateInst {
    Value _structValue; ///< The value of the struct being destructured.
    std::vector<Member> _members; ///< The members (fields) of the struct.

public:
    /// @brief Constructs a StructDestructureInst.
    ///
    /// @param structValue The struct value to destructure.
    /// @param members The list of struct members (fields).
    StructDestructureInst(Value structValue, std::vector<Member> members)
        : AggregateInst(InstKind::StructDestructureInstKind)
        , _structValue(structValue)
        , _members(std::move(members))
    {
    }

    /// @brief Gets the struct value operand.
    Value getStructValue() const { return _structValue; }

    /// @brief Gets the list of members (fields).
    std::vector<Member> const &getMembers() const { return _members; }

    /// @brief Gets the number of operands (always 1).
    size_t getOperandCount() const override { return 1; }

    /// @brief Gets the operand at the given index.
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return Operand(_structValue);
        llvm_unreachable("Invalid operand index");
    }

    /// @brief Gets the number of results (equal to number of struct fields).
    size_t getResultCount() const override { return _members.size(); }

    /// @brief Gets the result type at a given index (type of each field).
    Type getResultType(size_t index) const override
    {
        assert(index < _members.size() && "Result index out of range");
        return _members[index].getType();
    }

    /// @brief Checks if an instruction is a StructDestructureInst.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructDestructureInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_DESTRUCTURE_INST_HPP
