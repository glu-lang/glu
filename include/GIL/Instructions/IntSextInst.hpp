#ifndef GLU_GIL_INSTRUCTIONS_INT_SEXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_SEXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class IntSextInst
/// @brief Represents an instruction to sign-extend an integer value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to sign-extend an integer value in the GLU GIL (Generic Intermediate
/// Language).
class IntSextInst : public ConversionInst {
public:
    /// @brief Constructs an IntSextInst object.
    ///
    /// @param type The target type after zero extension.
    /// @param value The integer value to be zero-extended.
    IntSextInst(Type type, Value value)
        : ConversionInst(InstKind::IntSextInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntSextInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_SEXT_INST_HPP
