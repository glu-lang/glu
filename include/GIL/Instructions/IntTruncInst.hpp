#ifndef GLU_GIL_INSTRUCTIONS_INT_TRUNC_INST_HPP
#define GLU_GIL_INSTRUCTIONS_INT_TRUNC_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

/// @class IntTruncInst
/// @brief Represents an instruction to truncate an integer value.
///
/// This class is derived from ConversionInst and represents an instruction
/// to truncate an integer value in the GLU GIL (Generic Intermediate Language).
class IntTruncInst : public ConversionInst {
public:
    /// @brief Constructs an IntTruncInst object.
    ///
    /// @param type The target type after truncation.
    /// @param value The integer value to be truncated.
    IntTruncInst(Type type, Value value)
        : ConversionInst(InstKind::IntTruncInstKind, type, value)
    {
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_INT_TRUNC_INST_HPP
