#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class FloatExtInst
/// @brief Represents an instruction to extend a floating-point value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to extend a floating-point value in the GLU GIL (Generic Intermediate
/// Language).
class FloatExtInst : public ConversionInst {

public:
    /// @brief Constructs a FloatExtInst object.
    ///
    /// @param type The target type after extension.
    /// @param value The floating-point value to be extended.
    FloatExtInst(Type type, Value value)
        : ConversionInst(InstKind::FloatExtInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FloatExtInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP
