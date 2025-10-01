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
    Value _source; ///< The source value to borrow from
    Type _borrowType; ///< The type of the immutable reference

public:
    /// @brief Constructs an ImmutableBorrowInst object.
    /// @param source The source value to borrow from
    /// @param borrowType The type of the immutable reference
    ImmutableBorrowInst(Value source, Type borrowType)
        : OSSAInst(InstKind::ImmutableBorrowInstKind)
        , _source(source)
        , _borrowType(borrowType)
    {
    }

    /// @brief Gets the source value being borrowed from.
    /// @return The source value
    Value getSource() const { return _source; }

    /// @brief Gets the type of the immutable reference.
    /// @return The borrow type
    Type getBorrowType() const { return _borrowType; }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 1 (the immutable reference)
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
    /// @return The type of the immutable reference
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _borrowType;
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
