#ifndef GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP
#define GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class MutableBorrowInst
/// @brief Represents a mutable borrow instruction in OSSA (Ownership Static Single Assignment).
///
/// The mutable borrow instruction creates a mutable reference to a value without
/// taking ownership. The borrowed reference allows modification of the original value.
/// Only one mutable borrow can exist at a time for a given value, and no immutable
/// borrows can coexist with a mutable borrow.
///
/// Example GIL code:
/// @code
/// %1 = mutable_borrow %0
/// @endcode
///
/// This creates a mutable reference to %0 in %1. %0 becomes temporarily inaccessible
/// until the borrow ends.
class MutableBorrowInst : public OSSAInst {
    Value _source; ///< The source value to borrow from
    Type _borrowType; ///< The type of the mutable reference

public:
    /// @brief Constructs a MutableBorrowInst object.
    /// @param source The source value to borrow from
    /// @param borrowType The type of the mutable reference
    MutableBorrowInst(Value source, Type borrowType)
        : OSSAInst(InstKind::MutableBorrowInstKind), _source(source), _borrowType(borrowType)
    {
    }

    /// @brief Gets the source value being borrowed from.
    /// @return The source value
    Value getSource() const { return _source; }

    /// @brief Gets the type of the mutable reference.
    /// @return The borrow type
    Type getBorrowType() const { return _borrowType; }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 1 (the mutable reference)
    size_t getResultCount() const override { return 1; }

    /// @brief Returns the number of operands this instruction takes.
    /// @return Always 1 (the source value)
    size_t getOperandCount() const override { return 1; }

    /// @brief Gets the operand at the specified index.
    /// @param index The operand index (must be 0)
    /// @return The source value operand
    Operand getOperand(size_t index) const override
    {
        assert(index == 0 && "Invalid operand index");
        return _source;
    }

    /// @brief Gets the result type at the specified index.
    /// @param index The result index (must be 0)
    /// @return The type of the mutable reference
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _borrowType;
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a MutableBorrowInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a MutableBorrowInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::MutableBorrowInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_MUTABLE_BORROW_INST_HPP
