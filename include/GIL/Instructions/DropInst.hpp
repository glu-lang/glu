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
/// This destroys the value in %0 and releases its resources, making %0 invalid.
class DropInst : public OSSAInst {
public:
    /// @brief Constructs a DropInst object.
    /// @param value The value to drop
    DropInst(Value value) : OSSAInst(InstKind::DropInstKind, value) { }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 0 (drop produces no results)
    size_t getResultCount() const final override { return 0; }

    /// @brief Gets the result type at the specified index.
    /// @param index The result index (invalid for drop)
    /// @return Never returns (drop has no results)
    Type getResultType([[maybe_unused]] size_t index) const final override
    {
        llvm_unreachable("DropInst has no result type");
    }

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
