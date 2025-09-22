#ifndef GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STORE_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {
/// @class StoreInst
/// @brief A class representing a literal instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class StoreInst : public InstBase {
    Value source;
    Value dest;

public:
    StoreInst(Value source, Value dest)
        : InstBase(InstKind::StoreInstKind), source(source), dest(dest)
    {
    }

    Value getSource() const { return source; }
    Value getDest() const { return dest; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StoreInstKind;
    }

    Type getResultType([[maybe_unused]] size_t index) const override
    {
        llvm_unreachable("StoreInst has no result type");
    }
    size_t getResultCount() const override { return 0; }
    size_t getOperandCount() const override { return 2; }
    Operand getOperand([[maybe_unused]] size_t index) const override
    {
        if (index == 0)
            return source;
        if (index == 1)
            return dest;
        llvm_unreachable("Invalid operand index");
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STORE_INST_HPP
