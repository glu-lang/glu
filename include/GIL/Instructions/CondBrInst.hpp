#ifndef GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP

#include "TerminatorInst.hpp"

namespace glu::gil {
/// @class CondBrInst
/// @brief Represents a br instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
class CondBrInst : public TerminatorInst {
    Value condition;
    BasicBlock *thenBlock;
    BasicBlock *elseBlock;

public:
    CondBrInst(Value condition, BasicBlock *thenBlock, BasicBlock *elseBlock)
        : TerminatorInst(InstKind::CondBrInstKind), condition(condition), thenBlock(thenBlock), elseBlock(elseBlock)
    {
    }

    static bool classof(TerminatorInst const *inst)
    {
        return inst->getKind() == InstKind::CondBrInstKind;
    }

    size_t getOperandCount() const override { return 3; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        if (index == 0)
            return condition;
        if (index == 1)
            return thenBlock;
        if (index == 2)
            return elseBlock;
        else
            assert(false && "Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COND_BR_INST_HPP
