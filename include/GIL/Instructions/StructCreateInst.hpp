#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP

#include "AggregateInst.hpp"
#include <llvm/ADT/DenseMap.h>

namespace glu::gil {

/// @class StructCreateInst
/// @brief Represents an instruction to create a structure literal.
///
/// This class is derived from InstBase and represents an instruction
/// to create a structure literal in the GLU GIL (Generic Intermediate Language).
class StructCreateInst : public AggregateInst {

    Type _type; ///< The type of the structure.
    llvm::DenseMap<Member, Value> _operands; ///< The operands of the structure.

public:
    /// @brief Constructs a StructCreateInst object.
    ///
    /// @param _type The type of the structure.
    /// @param _operands The operands of the structure containing their name and
    ///                 their value.
    StructCreateInst(Type type, llvm::DenseMap<Member, Value> operands)
        : AggregateInst(InstKind::StructCreateInstKind)
        , _type(type)
        , _operands(std::move(operands))
    {
    }

    /// @brief Sets the type of the structure.
    ///
    /// @param type The new type of the structure.
    void setType(Type type) { this->_type = type; }

    /// @brief Gets the type of the structure.
    ///
    /// @return The type of the structure.
    Type getType() const { return _type; }

    /// @brief Sets the operands of the structure.
    ///
    /// @param operands The new operands of the structure.
    void setOperands(llvm::DenseMap<Member, Value> operands)
    {
        this->_operands = std::move(operands);
    }

    /// @brief Gets the operands of the structure.
    ///
    /// @return The operands of the structure.
    llvm::DenseMap<Member, Value> getOperands() const { return _operands; }

    size_t getOperandCount() const override { return _operands.size(); }
    Operand getOperand(size_t index) const override
    {
        assert(index < getOperandCount() && "Operand index out of range");
        if (index == 0) {
            return Operand(_type);
        } else {
            size_t current = 1;
            for (auto it : _operands) {
                if (current == index)
                    return it.first;
                current++;
            }
        }
    }

    size_t getResultCount() const override { return 1; }
    Type getResultType(size_t index) const override { return _type; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructCreateInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_CREATE_HPP
