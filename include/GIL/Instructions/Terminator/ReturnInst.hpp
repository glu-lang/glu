#ifndef GLU_GIL_INSTRUCTIONS_RETURN_INST_HPP
#define GLU_GIL_INSTRUCTIONS_RETURN_INST_HPP

#include "TerminatorInst.hpp"

namespace glu::gil {
/// @class ReturnInst
/// @brief Represents a return instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
class ReturnInst : public TerminatorInst {
    GLU_GIL_GEN_OPERAND(Value, Value, _value)

public:
    ReturnInst(Value value)
        : TerminatorInst(InstKind::ReturnInstKind), _value(value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::ReturnInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_RETURN_INST_HPP
