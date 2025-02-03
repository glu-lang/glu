#ifndef GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP

#include "ConversionInst.hpp"

namespace glu::gil {

class FloatExtInst : public ConversionInst {

public:
    FloatExtInst(Type type, Value value)
        : ConversionInst(InstKind::FloatExtInstKind, type, value)
    {
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_FLOAT_EXT_INST_HPP
