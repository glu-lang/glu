#ifndef GLU_GIL_INSTRUCTIONS_COPY_INST_HPP
#define GLU_GIL_INSTRUCTIONS_COPY_INST_HPP

#include "OSSAInst.hpp"

namespace glu::gil {

/// @class CopyInst
/// @brief Represents a copy instruction in OSSA (Ownership Static Single Assignment).
///
/// The copy instruction creates a copy of a value, leaving the original value intact.
/// Both the source and destination will have ownership of their respective values.
/// This operation is only valid for types that implement the Copy trait.
///
/// Example GIL code:
/// @code
/// %1 = copy %0
/// @endcode
/// 
/// This creates a copy of the value in %0 and stores it in %1, %0 remains valid.
class CopyInst : public OSSAInst {
    Value _source; ///< The source value to copy from

public:
    /// @brief Constructs a CopyInst object.
    /// @param source The source value to copy from
    CopyInst(Value source)
        : OSSAInst(InstKind::CopyInstKind), _source(source)
    {
    }

    /// @brief Gets the source value being copied.
    /// @return The source value
    Value getSource() const { return _source; }

    /// @brief Returns the number of results this instruction produces.
    /// @return Always 1 (the copied value)
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
    /// @return The type of the copied value (same as source type)
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _source.getType();
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is a CopyInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is a CopyInst
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::CopyInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_COPY_INST_HPP