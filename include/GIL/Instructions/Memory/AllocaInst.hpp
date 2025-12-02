#ifndef GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
#define GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP

#include "../InstBase.hpp"
#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"

namespace glu::gil {

///
///  @class AllocaInst
///  @brief Represents an allocation instruction.
///
///  This class is derived from InstBase and represents an allocation
///  instruction in the GLU GIL (Generic Intermediate Language).
///
class AllocaInst : public InstBase {
    using Context = glu::ast::ASTContext;
    GLU_GIL_GEN_OPERAND(PointeeType, Type, _pointeeType)
    Type _ptr; ///< The pointer type.

public:
    ///
    /// @brief Constructs an AllocaInst object.
    ///
    /// @param type The type of the allocation.
    /// @param context The AST Context.
    ///
    AllocaInst(Type pointeeType, Type pointerType)
        : InstBase(InstKind::AllocaInstKind)
        , _pointeeType(pointeeType)
        , _ptr(pointerType)
    {
    }

    Type getResultType() const { return _ptr; }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::AllocaInstKind;
    }
};

} // end namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ALLOCATION_HPP
