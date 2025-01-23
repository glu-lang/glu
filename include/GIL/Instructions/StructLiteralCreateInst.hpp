#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_LITERAL_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_LITERAL_CREATE_HPP

#include "ConstantInst.hpp"

namespace glu::gil {

/// @class StructLiteralInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from ConstantInst and represents an instruction
/// to create a structure literal in the GLU GIL (Generic Intermediate Language).
class StructLiteralInst : public ConstantInst {
protected:
    Type type; ///< The type of the structure.
    llvm::ArrayRef<Value> operands; ///< The operands of the structure.

public:
    /// @brief Constructs a StructLiteralInst object.
    ///
    /// @param type The type of the structure.
    /// @param operands The operands of the structure.
    StructLiteralInst(Type type, std::vector<Value> operands)
        : ConstantInst(InstKind::StructLiteralInstKind)
        , type(type)
        , operands(std::move(operands))
    {
    }

    /// @brief Sets the type of the structure.
    ///
    /// @param type The new type of the structure.
    void setType(Type type) { this->type = type; }

    /// @brief Gets the type of the structure.
    ///
    /// @return The type of the structure.
    Type getType() const { return type; }

    /// @brief Sets the operands of the structure.
    ///
    /// @param operands The new operands of the structure.
    void setOperands(std::vector<Value> operands)
    {
        this->operands = std::move(operands);
    }

    /// @brief Gets the operands of the structure.
    ///
    /// @return The operands of the structure.
    llvm::ArrayRef<Value> getOperands() const { return operands; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_LITERAL_CREATE_HPP
