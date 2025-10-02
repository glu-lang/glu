#ifndef GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP
#define GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class MutableBorrowInst
/// @brief Represents a mutable borrow instruction in OSSA (Ownership Static
/// Single Assignment).
///
/// The mutable borrow instruction creates a mutable reference to a value
/// without taking ownership. The borrowed reference allows modification of the
/// original value. Only one mutable borrow can exist at a time for a given
/// value, and no immutable borrows can coexist with a mutable borrow.
///
/// Example GIL code:
/// @code
/// %1 = mutable_borrow %0
/// @endcode
///
/// This creates a mutable reference to %0 in %1. %0 becomes temporarily
/// inaccessible until the borrow ends.
class MutableBorrowInst : public OSSAInst {
public:
    /// @brief Constructs a MutableBorrowInst object.
    /// @param source The source value to borrow from
    /// @param borrowType The type of the mutable reference
    MutableBorrowInst(Value source)
        : OSSAInst(InstKind::MutableBorrowInstKind, source)
    {
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a
    /// MutableBorrowInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a MutableBorrowInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::MutableBorrowInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP
