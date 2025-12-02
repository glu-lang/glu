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
    GLU_GIL_GEN_OPERAND(Member, Member, _member)

public:
    /// @brief Constructs an EnumVariantInst from a Member.
    ///
    /// @param member The enum variant to represent.
    EnumVariantInst(Member member)
        : ConstantInst(InstKind::EnumVariantInstKind), _member(member)
    {
        assert(
            llvm::isa<glu::types::EnumTy>(member.getType())
            && "Member must be of an enum type"
        );
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::EnumVariantInstKind;
    }

    Type getResultType() const { return _member.getType(); }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ENUM_VARIANT_INST_HPP
