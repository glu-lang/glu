#ifndef GLU_GIL_INSTRUCTIONS_DROP_INST_HPP
#define GLU_GIL_INSTRUCTIONS_DROP_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class DropInst
/// @brief Represents a drop instruction in OSSA (Ownership Static Single
/// Assignment).
///
/// The drop instruction explicitly destroys a value and releases any resources
/// it holds. After a drop, the value is no longer accessible. This instruction
/// is used to ensure deterministic destruction of values and proper resource
/// management in ownership-based type systems.
///
/// Example GIL code:
/// @code
/// drop %0
/// @endcode
///
/// This destroys the value pointed to by %0 and releases its resources,
/// taking ownership from the pointer location.
class DropInst : public OSSAInst {
public:
    /// @brief Constructs a DropInst object.
    /// @param ptr The pointer to the value to drop (accessed via getValue())
    DropInst(Value ptr) : OSSAInst(InstKind::DropInstKind, ptr) { }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a
    /// DropInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a DropInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::DropInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_DROP_INST_HPP
