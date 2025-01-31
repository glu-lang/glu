#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP

#include "InstBase.hpp"
#include <llvm/ADT/DenseMap.h>

namespace glu::gil {

/// @class StructCreateInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from InstBase and represents an instruction
/// to create a structure literal in the GLU GIL (Generic Intermediate Language).
class StructCreateInst : public InstBase {
protected:
    Type type; ///< The type of the structure.
    llvm::DenseMap<Value, std::string> operands; ///< The operands of the structure.

public:
    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param type The type of the structure.
    /// @param operands The operands of the structure containing their name and
    ///                 their value.
    StructCreateInst(Type type, llvm::DenseMap<Value, std::string> operands)
        : InstBase(InstKind::StructCreateInstKind)
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
    void setOperands(llvm::DenseMap<Value, std::string> operands)
    {
        this->operands = std::move(operands);
    }

    /// @brief Gets the operands of the structure.
    ///
    /// @return The operands of the structure.
    llvm::DenseMap<Value, std::string> getOperands() const { return operands; }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
