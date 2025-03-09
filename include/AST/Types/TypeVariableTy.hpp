#ifndef GLU_AST_TYPES_TYPEVARIABLETY_HPP
#define GLU_AST_TYPES_TYPEVARIABLETY_HPP

#include "TypeBase.hpp"

namespace glu::types {

class TypeVariableTy : public TypeBase {
public:
    /// @brief Constructor for the TypeVariableTy class.
    TypeVariableTy() : TypeBase(TypeKind::TypeVariableTyKind) { }

    /// @brief Static method to check if a type is a TypeVariableTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `TypeVariableTy`, `false`
    /// otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::TypeVariableTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_TYPEVARIABLETY_HPP
