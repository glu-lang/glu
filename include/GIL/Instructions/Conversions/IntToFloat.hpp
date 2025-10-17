#ifndef GLU_GIL_INSTRUCTIONS_CONVERSIONS_INT_TO_FLOAT_HPP
#define GLU_GIL_INSTRUCTIONS_CONVERSIONS_INT_TO_FLOAT_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

class IntToFloatInst : public ConversionInst {
public:
    IntToFloatInst(Value *src, Type *dstType)
        : ConversionInst(InstructionKind::IntToFloat, src, dstType)
    {
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CONVERSIONS_INT_TO_FLOAT_HPP
