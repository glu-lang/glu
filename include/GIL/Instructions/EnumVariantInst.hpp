#ifndef GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP

#include "ConstantInst.hpp"
#include "Member.hpp"

namespace glu::gil {

/// @class EnumVariantInst
/// @brief Represents a specific enum variant as an instruction.
///
/// This instruction is used to refer to a specific variant of an enum type.
/// It does not take any operands and simply holds a `Member` representing the
/// variant.
class EnumVariantInst : public ConstantInst {
    Member member; ///< The enum variant represented as a Member.

public:
    /// @brief Constructs an EnumVariantInst from a Member.
    ///
    /// @param member The enum variant to represent.
    EnumVariantInst(Member member)
        : ConstantInst(InstKind::EnumVariantInstKind), member(member)
    {
        // return member.getType();
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::EnumVariantInstKind;
    }

    Operand getOperand(size_t index) const override
    {
        assert(false && "EnumVariantInst has no operands");
    }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return member.getType();
    }

    Member getMember() const { return member; }
    void setMember(Member const &newMember) { member = newMember; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP
