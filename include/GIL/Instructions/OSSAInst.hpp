#ifndef GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP
#define GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

/// @class OSSAInst
/// @brief Base class for all OSSA (Ownership Static Single Assignment)
/// instructions.
///
/// This class serves as the base for all ownership-related instructions in the
/// GIL. OSSA instructions handle ownership semantics including moves, copies,
/// borrows, and resource management operations.
class OSSAInst : public InstBase {
public:
    /// @brief Constructs an OSSAInst object.
    /// @param kind The specific instruction kind
    OSSAInst(InstKind kind) : InstBase(kind) { }

    /// @brief Performs LLVM-style RTTI to check if an instruction is an
    /// OSSAInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is an OSSAInst or derived from it
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::OSSAInstFirstKind
            && inst->getKind() <= InstKind::OSSAInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP
