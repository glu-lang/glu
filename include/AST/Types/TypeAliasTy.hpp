#ifndef GLU_AST_TYPES_TYPEALIASTY_HPP
#define GLU_AST_TYPES_TYPEALIASTY_HPP

#include "SourceLocation.hpp"
#include "TypeBase.hpp"

namespace glu::types {

/// @brief TypeAliasTy is a class that represents the TypeAlias type in the AST.
class TypeAliasTy : public TypeBase {
    TypeBase * const _wrappedType;
    std::string _name;
    glu::SourceLocation _location;

public:
    /// @brief Constructor for the TypeAliasTy class.
    /// @param wrappedType The type being aliased.
    /// @param name The name of the alias.
    /// @param location The source location where the alias is defined.
    TypeAliasTy(
        TypeBase *wrappedType, std::string name, glu::SourceLocation location
    )
        : TypeBase(TypeKind::TypeAliasTyKind)
        , _wrappedType(wrappedType)
        , _name(std::move(name))
        , _location(location)
    {
    }

    /// @brief Getter for the wrapped type.
    /// @return The type being aliased.
    TypeBase *getWrappedType() const { return _wrappedType; }

    /// @brief Getter for the alias name.
    /// @return The name of the alias.
    std::string const &getName() const { return _name; }

    /// @brief Getter for the source location.
    /// @return The source location of the alias.
    glu::SourceLocation const &getLocation() const { return _location; }

    /// @brief Static method to check if a type is a TypeAliasTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `TypeAliasTy`, `false`
    /// otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::TypeAliasTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_TYPEALIASTY_HPP
