#ifndef GLU_AST_TYPES_CHARTY_HPP
#define GLU_AST_TYPES_CHARTY_HPP

#include "TypeBase.hpp"

#include <llvm/ADT/Hashing.h>

namespace glu::types {

/// @brief CharTy is a class that represents the char type in the AST.
class CharTy : public TypeBase {
public:
    /// @brief Constructor for the CharTy class.
    CharTy() : TypeBase(TypeKind::CharTyKind) { }

    /// @brief Static method to check if a type is a CharTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `CharTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::CharTyKind;
    }

    /// @brief Method to compare two CharTy.
    /// @param other The other CharTy to compare.
    /// @return Returns `true` if the two CharTy are equal, `false` otherwise.
    bool operator==(TypeBase const &other) const override
    {
        return other.getKind() == TypeKind::CharTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_CHARTY_HPP
