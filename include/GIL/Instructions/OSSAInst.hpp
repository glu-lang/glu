#ifndef GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP
#define GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP

#include "InstBase.hpp"

namespace glu::gil {

/// @class OSSAInst
/// @brief Base class for all OSSA (Ownership Static Single Assignment)
/// instructions.
///
/// This class serves as the base for all ownership-related instructions in the
/// GIL. OSSA instructions handle ownership semantics including moves, copies,
/// borrows, and resource management operations.
class OSSAInst : public InstBase {
    Value _source; ///< The source value for the OSSA instruction

public:
    /// @brief Constructs an OSSAInst object.
    /// @param kind The specific instruction kind
    OSSAInst(InstKind kind, Value source) : InstBase(kind), _source(source) { }

    size_t getOperandCount() const override { return 1; }
    size_t getResultCount() const override { return 1; }

    /// @brief Gets the source value for the OSSA instruction.
    Value getSource() const { return _source; }

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
    /// @return The type of the result (same as source type)
    Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Invalid result index");
        return _source.getType();
    }

    /// @brief Performs LLVM-style RTTI to check if an instruction is an
    /// OSSAInst.
    /// @param inst The instruction to check
    /// @return True if the instruction is an OSSAInst or derived from it
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::OSSAInstFirstKind
            && inst->getKind() <= InstKind::OSSAInstLastKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_OSSA_INST_HPP
