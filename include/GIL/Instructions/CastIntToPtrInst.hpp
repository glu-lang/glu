#ifndef GLU_GIL_CAST_INT_TO_PTR_INST_HPP
#define GLU_GIL_CAST_INT_TO_PTR_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

/// @class CastIntToPtrInst
/// @brief Represents an instruction to cast an integer to a pointer.
///
/// This class is derived from InstBase and represents an instruction
/// to cast an integer to a pointer in the GLU GIL (Generic Intermediate
/// Language).
class CastIntToPtrInst : public ConversionInst {

public:
    /// @brief Constructs a CastIntToPtrInst object.
    ///
    /// @param type The target pointer type.
    /// @param value The integer value to be casted.
    CastIntToPtrInst(Type type, Value value)
        : ConversionInst(InstKind::CastIntToPtrInstKind, type, value)
    {
    }

    /// @brief Checks if the given instruction is of type CastIntToPtrInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type CastIntToPtrInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CastIntToPtrInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_CAST_INT_TO_PTR_INST_HPP
