#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP

#include "AggregateInst.hpp"

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
class StructExtractInst : public AggregateInst {
    GLU_GIL_GEN_OPERAND(StructValue, Value, _structValue)
    GLU_GIL_GEN_OPERAND(Member, Member, _member)

public:
    ///
    /// @brief Constructs a StructExtractInst object.
    ///
    /// @param structValue The structure value from which to extract the member.
    /// @param member The member to extract from the structure.
    ///
    StructExtractInst(Value structValue, Member member)
        : AggregateInst(InstKind::StructExtractInstKind)
        , _structValue(structValue)
        , _member(member)
    {
    }

    virtual size_t getResultCount() const override { return 1; }

    /// @brief Returns the type of the result at the specified index.
    /// @param index The index of the result.
    /// @return The type of the result at the specified index.
    virtual Type getResultType(size_t index) const override
    {
        assert(index < getResultCount() && "Result index out of range");
        return _member.getType();
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructExtractInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
