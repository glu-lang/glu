#ifndef GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

class CastPtrToIntInst : public ConversionInst {

public:
    /// @brief Constructs a CastPtrToIntInst object.
    ///
    /// @param type The target integer type.
    /// @param value The pointer value to be casted.
    CastPtrToIntInst(Type type, Value value)
        : ConversionInst(InstKind::CastPtrToIntInstKind, type, value)
    {
    }

    /// @brief Checks if the given instruction is of type CastPtrToIntInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type CastPtrToIntInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CastPtrToIntInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CAST_PTR_TO_INT_INST_HPP
