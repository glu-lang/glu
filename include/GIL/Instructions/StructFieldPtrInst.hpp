#ifndef GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP
#define GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AggregateInst.hpp"

namespace glu::gil {

/// @class StructFieldPtrInst
/// @brief Represents an instruction that creates a pointer to a field within a
/// struct.
///
/// This class is derived from InstBase and represents an instruction that
/// calculates and returns a pointer to a specific field of a structure in the
/// GLU GIL (Generic Intermediate Language).
class StructFieldPtrInst : public AggregateInst {
    Member _member; ///< The member descriptor for the field
    Type _ptr; ///< The pointer type to the field

public:
    /// @brief Constructs a StructFieldPtrInst object.
    ///
    /// Creates an instruction that computes a pointer to a field within a
    /// struct.
    ///
    /// @param structValue The value of the struct being accessed
    /// @param member The descriptor of the field being accessed
    /// @param context The AST context used to allocate memory for the pointer
    /// type
    StructFieldPtrInst(Member member, glu::ast::ASTContext *context)
        : AggregateInst(InstKind::StructFieldPtrInstKind)
        , _member(member)
        , _ptr(Type(
              // #TODO: Use context to deduce size and alignement of the pointer
              // type
              sizeof(void *), alignof(void *), false,
              context->getTypesMemoryArena().allocate<glu::types::PointerTy>(
                  _member.getType().getType()
              )
          ))
    {
    }

    /// @brief Gets the member descriptor.
    ///
    /// @return The descriptor of the field being accessed.
    Member getMember() const { return _member; }

    /// @brief Gets the number of results produced by this instruction.
    ///
    /// @return The number of results (always 1 - the field pointer).
    virtual size_t getResultCount() const override { return 1; }

    /// @brief Gets the number of operands required by this instruction.
    ///
    /// @return The number of operands (always 2 - struct and member).
    virtual size_t getOperandCount() const override { return 1; }

    /// @brief Gets the operand at the specified index.
    ///
    /// @param index The index of the operand (0 for struct, 1 for member).
    /// @return The operand at the specified index.
    virtual Operand getOperand(size_t index) const override
    {
        if (index == 0)
            return Operand(_member);
        llvm_unreachable("Invalid operand index");
    }

    /// @brief Gets the result type at the specified index.
    ///
    /// @param index The index of the result type (must be 0).
    /// @return The result type at the specified index (pointer to the field
    /// type).
    virtual Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _ptr;
    }

    /// @brief Checks if the given instruction is of type StructFieldPtrInst.
    ///
    /// @param inst The instruction to check.
    /// @return True if the instruction is of type StructFieldPtrInst, false
    /// otherwise.
    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::StructFieldPtrInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_STRUCT_FIELD_PTR_INST_HPP
