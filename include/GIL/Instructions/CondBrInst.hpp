#ifndef GLU_GIL_INSTRUCTIONS_BR_COND_INST_HPP
#define GLU_GIL_INSTRUCTIONS_BR_COND_INST_HPP

#include "TerminatorInst.hpp"

namespace glu::gil {
/// @class BrCondInst
/// @brief Represents a br instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
class BrCondInst : public TerminatorInst {
    BoolTy condition;
    BasicBlock thenBlock;
    BasicBlock elseBlock;

public:
    BrCondInst(BoolTy condition, BasicBlock thenBlock, BasicBlock elseBlock)
        : TerminatorInst(InstKind::BrCondInstKind), condition(condition), thenBlock(thenBlock), elseBlock(elseBlock)
    {
    }

    static bool classof(TerminatorInst const *inst)
    {
        return inst->getKind() == InstKind::BrCondInstKind;
    }

    size_t getOperandCount() const override { return 0; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return value;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_BR_COND_INST_HPP
