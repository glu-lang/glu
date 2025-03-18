#ifndef GLU_GIL_FUNCTION_PTR_INST_HPP
#define GLU_GIL_FUNCTION_PTR_INST_HPP

#include "Function.hpp"

namespace glu::gil {

/// @class FunctionPtrInst
/// @brief Represents an instruction that creates and holds a pointer to a
///        function.
///
/// This class is derived from InstBase and represents an instruction
/// to hold a pointer to a function in the GLU GIL (Generic Intermediate
/// Language).
class FunctionPtrInst : public InstBase {
    Function *_function; ///< The function pointer.
    Type _type; ///< The type of the function pointer.

public:
    /// @brief Constructs a FunctionPtrInst object.
    ///
    /// @param function The function pointer.
    /// @param type The type of the function pointer.
    FunctionPtrInst(Function *function, Type type)
        : InstBase(InstKind::FunctionPtrInstKind)
        , _function(function)
        , _type(type)
    {
        assert(function && "FunctionPtrInst requires a valid function pointer");
    }

    /// @brief Gets the number of operands.
    ///
    /// @return The number of operands.
    size_t getOperandCount() const override { return 2; }

    /// @brief Gets the operand at the specified index.
    ///
    /// @param index The index of the operand.
    /// @return The operand at the specified index.
    Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return Operand(_function);
        if (index == 1)
            return Operand(_type);
        llvm_unreachable("Invalid operand index");
    }

    /// @brief Gets the number of results.
    ///
    /// @return The number of results.
    size_t getResultCount() const override { return 1; }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type.
    /// @return The result type at the specified index.
    Type getResultType([[maybe_unused]] size_t index) const override
    {
        return _type;
    }

    /// @brief Gets the function pointer.
    ///
    /// @return The function pointer.
    Function *getFunction() const { return _function; }

    /// @brief Checks if the given instruction is of type FunctionPtrInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type FunctionPtrInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FunctionPtrInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_FUNCTION_PTR_INST_HPP
