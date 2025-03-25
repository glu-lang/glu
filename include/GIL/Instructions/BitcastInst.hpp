#ifndef GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP
#define GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class BitcastInst
/// @brief Represents an instruction to extend a floating-point value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to extend a floating-point value in the GLU GIL (Generic Intermediate
/// Language).
class BitcastInst : public ConversionInst {

public:
    /// @brief Constructs a BitcastInst object.
    ///
    /// @param type The target type after extension.
    /// @param value The floating-point value to be extended.
    BitcastInst(Type type, Value value)
        : ConversionInst(InstKind::BitcastInstKind, type, value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::BitcastInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BITCAST_INST_HPP
