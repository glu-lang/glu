#ifndef GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class intZextInst
/// @brief Represents an instruction to zero-extend an integer value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to zero-extend an integer value in the GLU GIL (Generic Intermediate Language).
class intZextInst : public ConversionInst {
public:
    /// @brief Constructs an intZextInst object.
    ///
    /// @param type The target type after zero extension.
    /// @param value The integer value to be zero-extended.
    intZextInst(Type type, Value value)
        : ConversionInst(InstKind::intZextInstKind, type, value)
    {
    }

    static bool classof(ConversionInst const *inst)
    {
        return inst->getKind() == InstKind::intZextInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_ZEXT_INST_HPP