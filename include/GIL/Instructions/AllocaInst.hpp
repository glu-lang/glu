#ifndef GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
#define GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "InstBase.hpp"
#include "Type.hpp"

namespace glu::gil {

///
///  @class AllocaInst
///  @brief Represents an allocation instruction.
///
///  This class is derived from InstBase and represents an allocation
///  instruction in the GLU GIL (Generic Intermediate Language).
///
class AllocaInst : public InstBase {
    using TypeMemory = glu::TypedMemoryArena<glu::ast::ASTNode>;
    Type _type; ///< The type of the allocation.
    TypeMemory *_typeMemory; ///< The Typed Memory Arena.
    Type _ptr; ///< The pointer type.
public:
    ///
    /// @brief Constructs an AllocaInst object.
    ///
    /// @param type The type of the allocation.
    /// @param astTypeMemory The Typed Memory Arena.
    ///
    AllocaInst(Type type, TypeMemory *astTypeMemory)
        : InstBase(InstKind::AllocaInstKind)
        , _type(type)
        , _typeMemory(astTypeMemory)
        , _ptr(Type(
              sizeof(void *), alignof(void *), false,
              _typeMemory->allocate<glu::types::PointerTy>(_type.getType())
          ))
    {
    }

    ///
    /// @brief Gets the type of the allocation.
    ///
    /// @return The type of the allocation.
    ///
    Type getType() const { return _type; }

    virtual size_t getResultCount() const override { return 1; }

    virtual size_t getOperandCount() const override { return 1; }

    virtual Operand getOperand(size_t index) const override
    {
        assert(index == 0 && "Operand index out of range");
        return Operand(_type);
    }

    virtual Type getResultType(size_t index) const override
    {
        assert(index == 0 && "Result index out of range");
        return _ptr;
    }

    bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::AllocaInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
