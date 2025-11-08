#ifndef GLU_GIL_INSTRUCTIONS_CONSTANT_INST_HPP
#define GLU_GIL_INSTRUCTIONS_CONSTANT_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {
/// @class ConstantInst
/// @brief A class representing a literal instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class ConstantInst : public InstBase {
public:
    ConstantInst(InstKind kind) : InstBase(kind) { }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::ConstantInstFirstKind
            && inst->getKind() <= InstKind::ConstantInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_CONSTANT_INST_HPP
