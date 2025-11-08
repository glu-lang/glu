#ifndef GLU_GIL_FUNCTION_PTR_INST_HPP
#define GLU_GIL_FUNCTION_PTR_INST_HPP

#include "ConstantInst.hpp"
#include "Function.hpp"

namespace glu::gil {

/// @class FunctionPtrInst
/// @brief Represents an instruction that creates and holds a pointer to a
///        function.
///
/// This class is derived from InstBase and represents an instruction
/// to hold a pointer to a function in the GLU GIL (Generic Intermediate
/// Language).
class FunctionPtrInst : public ConstantInst {
    GLU_GIL_GEN_OPERAND(Function, Function *, _function)
    GLU_GIL_GEN_OPERAND(Type, Type, _type)

public:
    /// @brief Constructs a FunctionPtrInst object.
    ///
    /// @param function The function pointer.
    /// @param type The type of the function pointer.
    FunctionPtrInst(Function *function, Type type)
        : ConstantInst(InstKind::FunctionPtrInstKind)
        , _function(function)
        , _type(type)
    {
        assert(function && "FunctionPtrInst requires a valid function pointer");
    }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type.
    /// @return The result type at the specified index.
    Type getResultType([[maybe_unused]] size_t index) const override
    {
        return _type;
    }

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
