#ifndef GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class IntZextInst
/// @brief Represents an instruction to zero-extend an integer value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to zero-extend an integer value in the GLU GIL (Generic Intermediate
/// Language).
class IntZextInst : public ConversionInst {
public:
    /// @brief Constructs an IntZextInst object.
    ///
    /// @param type The target type after zero extension.
    /// @param value The integer value to be zero-extended.
    IntZextInst(Type type, Value value)
        : ConversionInst(InstKind::IntZextInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntZextInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP
