#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_TRUNC_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_TRUNC_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class FloatTruncInst
/// @brief Represents an instruction to extend a floating-point value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to extend a floating-point value in the GLU GIL (Generic Intermediate
/// Language).
class FloatTruncInst : public ConversionInst {

public:
    /// @brief Constructs a FloatTruncInst object.
    ///
    /// @param type The target type after extension.
    /// @param value The floating-point value to be extended.
    FloatTruncInst(Type type, Value value)
        : ConversionInst(InstKind::FloatTruncInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FloatTruncInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_TRUNC_INST_HPP
