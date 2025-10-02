#ifndef GLU_GIL_INSTRUCTIONS_MOVE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_MOVE_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class MoveInst
/// @brief Represents a move instruction in OSSA (Ownership Static Single
/// Assignment).
///
/// The move instruction transfers ownership of a value from one location to
/// another. After a move, the source value is no longer accessible and the
/// destination owns the value. This is a fundamental operation in
/// ownership-based type systems.
///
/// Example GIL code:
/// @code
/// %1 = move %0
/// @endcode
///
/// This moves the value from %0 to %1, invalidating %0.
class MoveInst : public OSSAInst {
public:
    /// @brief Constructs a MoveInst object.
    /// @param source The source value to move from
    MoveInst(Value source) : OSSAInst(InstKind::MoveInstKind, source) { }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a
    /// MoveInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a MoveInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::MoveInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_MOVE_INST_HPP
