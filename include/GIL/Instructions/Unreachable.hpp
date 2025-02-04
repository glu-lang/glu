#ifndef GLU_GIL_INSTRUCTIONS_UNREACHABLE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_UNREACHABLE_INST_HPP

#include "TerminatorInst.hpp"

namespace glu::gil {
/// @class ReturnInst
/// @brief Represents a return instruction in the GIL.
///
/// This instruction is a control flow terminator, meaning it marks the end of
/// execution in a function. It does not produce any results and must always be
/// the last instruction in a basic block.
class ReturnInst : public TerminatorInst {
public:
    ReturnInst(Value value)
        : TerminatorInst(InstKind::ReturnInstKind), value(value)
    {
        // TODO: assert(llvm::isa<glu::types::PointerTy>(*value.getType()));
    }

    static bool classof(TerminatorInst const *inst)
    {
        return inst->getKind() == InstKind::ReturnInstKind;
    }

    size_t getOperandCount() const override { return 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        llvm_unreachable("No operand");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_UNREACHABLE_INST_HPP
