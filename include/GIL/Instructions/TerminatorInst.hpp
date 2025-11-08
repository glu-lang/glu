#ifndef GLU_GIL_INSTRUCTIONS_TERMINATOR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_TERMINATOR_INST_HPP

#include "InstBase.hpp"

// Forward declarations
namespace glu::gil {
/// @class TerminatorInst
/// @brief A class representing a terminator instruction in the GIL.
///
/// These instructions are used to control the flow of execution in a function.
/// They have no results and are always the last instruction in a basic block.
class TerminatorInst : public InstBase {
public:
    TerminatorInst(InstKind kind) : InstBase(kind) { }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::TerminatorInstFirstKind
            && inst->getKind() <= InstKind::TerminatorInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_TERMINATOR_INST_HPP
