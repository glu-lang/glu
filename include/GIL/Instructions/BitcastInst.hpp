#ifndef GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP
#define GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class BitCast
/// @brief Represents an instruction to extend a floating-point value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to extend a floating-point value in the GLU GIL (Generic Intermediate
/// Language).
class BitCast : public ConversionInst {

public:
    /// @brief Constructs a BitCast object.
    ///
    /// @param type The target type after extension.
    /// @param value The floating-point value to be extended.
    BitCast(Type type, Value value)
        : ConversionInst(InstKind::BitCastKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::BitcastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP
