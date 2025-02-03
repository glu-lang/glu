#ifndef GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP
#define GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {
/// @class LoadInst
/// @brief A class representing a literal instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class LoadInst : public InstBase {
private:
    Value value;

public:
    LoadInst(Value value) : value(value)
    {
        assert(llvm::isa<glu::types::PointerTy>(*value.getType()));
    }

    Value getValue() const { return value; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::LoadInstKind;
    }

    size_t getResultCount() const override { return 1; }
    size_t getOperandCount() const override { return 1; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return value
    }
    Type getResultType(size_t index) const override
    {
        // TODO: return
        // llvm::dyn_cast<PointerTy>(*value.getType())->getPointee();
        return Type();
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_LOAD_INST_HPP
