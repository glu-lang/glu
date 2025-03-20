#ifndef GLU_AST_TYPES_VOIDTY_HPP
#define GLU_AST_TYPES_VOIDTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief VoidTy is a class that represents an unkown type
/// in the AST.
class VoidTy : public TypeBase {

public:
    VoidTy() : TypeBase(TypeKind::VoidTyKind) { }

    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::VoidTyKind;
    }
};

}

#endif // GLU_AST_TYPES_VOIDTY_HPP
