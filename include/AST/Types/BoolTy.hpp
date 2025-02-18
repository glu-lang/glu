#ifndef GLU_AST_TYPES_BOOLTY_HPP
#define GLU_AST_TYPES_BOOLTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief BoolTy is a class that represents the bool type in the AST.
class BoolTy : public TypeBase {
public:
    /// @brief Constructor for the BoolTy class.
    BoolTy() : TypeBase(TypeKind::BoolTyKind) { }

    /// @brief Static method to check if a type is a BoolTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `BoolTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::BoolTyKind;
    }

    /// @brief Method to hash the BoolTy.
    /// @return Returns the hash of the BoolTy.
    std::size_t hash() const override { return 0; }

    /// @brief Method to compare two BoolTy.
    /// @param other The other BoolTy to compare.
    /// @return Returns `true` if the two BoolTy are equal, `false` otherwise.
    bool operator==(TypeBase const &other) const override
    {
        return other.getKind() == TypeKind::BoolTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_BOOLTY_HPP
