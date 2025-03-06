#ifndef GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class intSextInst
/// @brief Represents an instruction to zero-extend an integer value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to zero-extend an integer value in the GLU GIL (Generic Intermediate Language).
class intSextInst : public ConversionInst {
public:
    /// @brief Constructs an intSextInst object.
    ///
    /// @param type The target type after zero extension.
    /// @param value The integer value to be zero-extended.
    intSextInst(Type type, Value value)
        : ConversionInst(InstKind::intSextInstKind, type, value)
    {
    }

    static bool classof(ConversionInst const *inst)
    {
        return inst->getKind() == InstKind::intSextInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP