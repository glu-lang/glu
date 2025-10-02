#ifndef GLU_GIL_INSTRUCTIONS_IMMUTABLE_BORROW_INST_HPP
#define GLU_GIL_INSTRUCTIONS_IMMUTABLE_BORROW_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class ImmutableBorrowInst
/// @brief Represents an immutable borrow instruction in OSSA (Ownership Static
/// Single Assignment).
///
/// The immutable borrow instruction creates an immutable reference to a value
/// without taking ownership. The borrowed reference allows read-only access to
/// the original value. Multiple immutable borrows can exist simultaneously for
/// the same value, but no mutable borrows can coexist with immutable borrows.
///
/// Example GIL code:
/// @code
/// %1 = immutable_borrow %0
/// @endcode
///
/// This creates an immutable reference to %0 in %1. %0 can still be accessed
/// through other immutable borrows but cannot be mutated.
class ImmutableBorrowInst : public OSSAInst {
public:
    /// @brief Constructs an ImmutableBorrowInst object.
    /// @param source The source value to borrow from
    ImmutableBorrowInst(Value source)
        : OSSAInst(InstKind::ImmutableBorrowInstKind, source)
    {
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is an
    /// ImmutableBorrowInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is an ImmutableBorrowInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::ImmutableBorrowInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_IMMUTABLE_BORROW_INST_HPP
