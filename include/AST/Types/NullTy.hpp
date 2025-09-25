#ifndef GLU_AST_TYPES_NULLTY_HPP
#define GLU_AST_TYPES_NULLTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief NullTy is a class that represents the null type in the AST.
class NullTy : public TypeBase {
public:
    /// @brief Constructor for the NullTy class.
    NullTy() : TypeBase(TypeKind::NullTyKind) { }

    /// @brief Static method to check if a type is a NullTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `NullTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::NullTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_NULLTY_HPP
