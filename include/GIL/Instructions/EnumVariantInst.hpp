#ifndef GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP

#include "InstBase.hpp"
#include "Member.hpp"

namespace glu::gil {

/// @class EnumVariantInst
/// @brief Represents a specific enum variant as an instruction.
///
/// This instruction is used to refer to a specific variant of an enum type.
/// It does not take any operands and simply holds a `Member` representing the
/// variant.
class EnumVariantInst : public InstBase {
    Member member; ///< The enum variant represented as a Member.

public:
    /// @brief Constructs an EnumVariantInst from a Member.
    ///
    /// @param member The enum variant to represent.
    EnumVariantInst(Member member)
        : InstBase(InstKind::EnumVariantInstKind), member(member)
    {
        return member.getType();
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::EnumVariantInstKind;
    }

    size_t getResultCount() const override { return 1; }
    size_t getOperandCount() const override { return 0; }

    Operand getOperand(size_t index) const override
    {
        assert(false && "EnumVariantInst has no operands");
        return Operand();
    }

    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return member.getType();
    }

    Member getMember() const { return member; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP
