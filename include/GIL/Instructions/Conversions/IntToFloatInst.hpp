#ifndef GLU_GIL_INSTRUCTIONS_INT_TO_FLOAT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_TO_FLOAT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class IntToFloatInst
/// @brief Represents an instruction to convert an integer value to a
/// floating-point value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to convert an integer value to a floating-point value in the GLU GIL
/// (Generic Intermediate Language).
class IntToFloatInst : public ConversionInst {
public:
    /// @brief Constructs an IntToFloatInst object.
    ///
    /// @param type The target floating-point type after conversion.
    /// @param value The integer value to be converted.
    IntToFloatInst(Type type, Value value)
        : ConversionInst(InstKind::IntToFloatInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::IntToFloatInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_TO_FLOAT_INST_HPP
