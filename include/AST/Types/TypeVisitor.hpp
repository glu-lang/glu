#ifndef GLU_AST_TYPES_TYPEVISITOR_HPP
#define GLU_AST_TYPES_TYPEVISITOR_HPP

#include "TypeBase.hpp"
#include <llvm/Support/ErrorHandling.h>

namespace glu::types {

template <typename Impl, typename RetTy = void, typename... ArgTys>
class TypeVisitor {
    Impl *asImpl() { return static_cast<Impl *>(this); }

public:
    /**
     * @brief Visit a Type.
     *
     * This function call the appropriate visit function for the type's kind.
     *
     * @param type The type to visit.
     * @param ...args The arguments to pass to the visit function.
     * @return The value returned by the visit function.
     *
     **/
    RetTy visit(TypeBase *type, ArgTys... args)
    {
        return _visitType(type, std::forward<ArgTys>(args)...);
    }

    RetTy _visitType(TypeBase *type, ArgTys... args)
    {
        switch (type->getKind()) {
#define TYPE(NAME)                                                     \
case TypeKind::NAME##Kind:                                             \
    return asImpl()->visit##NAME(type, std::forward<ArgTys>(args)...);
#include "TypeKind.def"
        default: llvm_unreachable("Unknown type kind.");
        }
    }

    RetTy visitTypeBase(TypeBase *type, ArgTys... args) { }

#define TYPE(NAME)                                                         \
    RetTy visit##NAME(TypeBase *type, ArgTys... args)                      \
    {                                                                      \
        return asImpl()->visit##NAME(type, std::forward<ArgTys>(args)...); \
    }
#include "TypeKind.def"
};

} // namespace glu::types

#endif /* !GLU_AST_TYPES_TYPEVISITOR_HPP */
