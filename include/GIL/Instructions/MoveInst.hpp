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
    Value _source; ///< The source value to move from

public:
    /// @brief Constructs a MoveInst object.
    /// @param source The source value to move from
    MoveInst(Value source) : OSSAInst(InstKind::MoveInstKind), _source(source)
    {
    }

    /// @brief Gets the source value being moved.
    /// @return The source value
    Value getSource() const { return _source; }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 1 (the moved value)
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
    /// @return The type of the moved value (same as source type)
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _source.getType();
    }

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
