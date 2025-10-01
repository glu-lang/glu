#ifndef GLU_GIL_INSTRUCTIONS_END_BORROW_INST_HPP
#define GLU_GIL_INSTRUCTIONS_END_BORROW_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class EndBorrowInst
/// @brief Represents an end borrow instruction in OSSA (Ownership Static Single
/// Assignment).
///
/// The end borrow instruction explicitly ends a borrow scope and returns
/// ownership of the value to its original location. After an end_borrow, the
/// borrowed reference is no longer valid and the original value becomes
/// accessible again.
///
/// Example GIL code:
/// @code
/// end_borrow %1 from %0
/// @endcode
///
/// This ends the borrow represented by %1 and returns full access to %0.
/// After this instruction, %1 is invalid and %0 can be used normally.
class EndBorrowInst : public OSSAInst {
    Value _borrowedValue; ///< The borrowed reference to end
    Value _originalValue; ///< The original value that was borrowed from

public:
    /// @brief Constructs an EndBorrowInst object.
    /// @param borrowedValue The borrowed reference to end
    /// @param originalValue The original value that was borrowed from
    EndBorrowInst(Value borrowedValue, Value originalValue)
        : OSSAInst(InstKind::EndBorrowInstKind)
        , _borrowedValue(borrowedValue)
        , _originalValue(originalValue)
    {
    }

    /// @brief Gets the borrowed reference being ended.
    /// @return The borrowed value
    Value getBorrowedValue() const { return _borrowedValue; }

    /// @brief Gets the original value that was borrowed from.
    /// @return The original value
    Value getOriginalValue() const { return _originalValue; }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 0 (end_borrow produces no results)
    size_t getResultCount() const override { return 0; }

    /// @brief Returns the number of operands this instruction takes.
    /// @return Always 2 (the borrowed value and original value)
    size_t getOperandCount() const override { return 2; }

    /// @brief Gets the operand at the specified index.
    /// @param index The operand index (0 = borrowed value, 1 = original value)
    /// @return The requested operand
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return _borrowedValue;
        if (index == 1)
            return _originalValue;
        llvm_unreachable("Invalid operand index");
    }

    /// @brief Gets the result type at the specified index.
    /// @param index The result index (invalid for end_borrow)
    /// @return Never returns (end_borrow has no results)
    Type getResultType([[maybe_unused]] size_t index) const override
    {
        llvm_unreachable("EndBorrowInst has no result type");
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is an
    /// EndBorrowInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is an EndBorrowInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::EndBorrowInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_END_BORROW_INST_HPP
