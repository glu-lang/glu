#ifndef GLU_GIL_INSTRUCTIONS_BR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_BR_INST_HPP

#include "TerminatorInst.hpp"

namespace glu::gil {
/// @class BrInst
/// @brief Represents a br instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
class BrInst : public TerminatorInst {
    BasicBlock *value;

public:
    BrInst(BasicBlock *value)
        : TerminatorInst(InstKind::BrInstKind), value(value)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::BrInstKind;
    }

    size_t getOperandCount() const override { return 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return value;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BR_INST_HPP
