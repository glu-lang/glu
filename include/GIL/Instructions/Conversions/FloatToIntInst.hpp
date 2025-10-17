#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_TO_INT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_TO_INT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class FloatToIntInst
/// @brief Represents an instruction to convert a floating-point value to an
/// integer.
///
/// This class is derived from ConversionInst and represents an instruction
/// to convert a floating-point value to an integer value in the GLU GIL
/// (Generic Intermediate Language).
class FloatToIntInst : public ConversionInst {
public:
    /// @brief Constructs a FloatToIntInst object.
    ///
    /// @param type The target integer type after conversion.
    /// @param value The floating-point value to be converted.
    FloatToIntInst(Type type, Value value)
        : ConversionInst(InstKind::FloatToIntInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FloatToIntInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_TO_INT_INST_HPP
