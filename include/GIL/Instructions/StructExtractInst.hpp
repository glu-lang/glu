#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP

#include "InstBase.hpp"

namespace glu::gil {

/**
 * @class StructExtractInst
 * @brief Represents an instruction to extract a member from a structure.
 *
 * This class is derived from InstBase and represents an instruction to extract
 * a member from a structure in the GLU GIL (Generic Intermediate Language).
 */
class StructExtractInst : public InstBase {
    Value
        structValue; ///< The structure value from which to extract the member.
    Member member; ///< The member to extract from the structure.

public:
    /**
     * @brief Constructs a StructExtractInst object.
     *
     * @param structValue The structure value from which to extract the member.
     * @param member The member to extract from the structure.
     */
    StructExtractInst(Value structValue, Member member)
        : InstBase(InstKind::StructExtractInstKind)
        , structValue(structValue)
        , member(member)
    {
    }

    /**
     * @brief Gets the structure value.
     *
     * @return The structure value.
     */
    Value getStructValue() const { return structValue; }

    /**
     * @brief Gets the member to extract.
     *
     * @return The member to extract.
     */
    Member getMember() const { return member; }

    // TODO: Implement getMemberType() method when Memeber will be defined.
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_EXTRACT_HPP
