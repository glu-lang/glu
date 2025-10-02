#ifndef GLU_GIL_INSTRUCTIONS_COPY_INST_HPP
#define GLU_GIL_INSTRUCTIONS_COPY_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class CopyInst
/// @brief Represents a copy instruction in OSSA (Ownership Static Single
/// Assignment).
///
/// The copy instruction creates a copy of a value, leaving the original value
/// intact. Both the source and destination will have ownership of their
/// respective values. This operation is only valid for types that implement the
/// Copy trait.
///
/// Example GIL code:
/// @code
/// %1 = copy %0
/// @endcode
///
/// This creates a copy of the value in %0 and stores it in %1, %0 remains
/// valid.
class CopyInst : public OSSAInst {

public:
    /// @brief Constructs a CopyInst object.
    /// @param source The source value to copy from
    CopyInst(Value source) : OSSAInst(InstKind::CopyInstKind, source) { }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a
    /// CopyInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a CopyInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CopyInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COPY_INST_HPP
