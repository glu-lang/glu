#ifndef GLU_AST_TYPES_TYPEVARIABLETY_HPP
#define GLU_AST_TYPES_TYPEVARIABLETY_HPP

#include "TypeBase.hpp"

namespace glu::types {

class TypeVariableTy : public TypeBase {

    unsigned _id = 0; ///< Unique identifier for the type variable.

public:
    /// @brief Constructor for the TypeVariableTy class.
    TypeVariableTy() : TypeBase(TypeKind::TypeVariableTyKind) { }

    /// @brief Constructor for the TypeVariableTy class with an ID.
    /// @param id The unique identifier for the type variable.
    TypeVariableTy(unsigned id)
        : TypeBase(TypeKind::TypeVariableTyKind), _id(id)
    {
    }

    /// @brief Sets the unique identifier for the type variable.
    /// @param id The unique identifier for the type variable.
    void setID(unsigned id) { _id = id; }

    /// @brief Gets the unique identifier for the type variable.
    /// @return The unique identifier for the type variable.
    unsigned getID() const { return _id; }

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
