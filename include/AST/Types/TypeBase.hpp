#ifndef GLU_AST_TYPES_TYPEBASE_HPP
#define GLU_AST_TYPES_TYPEBASE_HPP

#include <llvm/Support/Casting.h>
#include <string>
#include <llvm/ADT/StringRef.h>

namespace glu::types {

/// @brief Discriminator for LLVM-style RTTI (used in dyn_cast<> and similar
/// operations)
enum class TypeKind {
#define TYPE(NAME) NAME##Kind,
#include "Types/TypeKind.def"
};

/// @brief Converts a TypeKind enum value to a readable string representation.
/// @param kind The TypeKind to convert.
/// @return A std::string representing the TypeKind.
inline std::string toString(glu::types::TypeKind kind)
{
    llvm::StringRef kindStr;

    switch (kind) {
#define TYPE(Name)                                                 \
    case glu::types::TypeKind::Name##Kind: kindStr = #Name; break;
#include "Types/TypeKind.def"
    default: return "Unknown";
    }

    kindStr.consume_back("Ty");
    return kindStr.str();
}

/// @brief Enables string concatenation with a TypeKind using the + operator.
/// @param lhs A std::string to concatenate with.
/// @param kind A TypeKind to convert and append.
/// @return The resulting concatenated string.
inline std::string operator+(std::string const &lhs, glu::types::TypeKind kind)
{
    return lhs + toString(kind);
}

/// @brief Enables string concatenation with a TypeKind in reverse order.
/// @param kind A TypeKind to convert and prepend.
/// @param rhs A std::string to append to.
/// @return The resulting concatenated string.
inline std::string operator+(glu::types::TypeKind kind, std::string const &rhs)
{
    return toString(kind) + rhs;
}

/// @brief Base class for every Type definition. Contains the most basic
///        elements each types should at least have.
class TypeBase {
    TypeKind const _kind;

public:
    /// @brief Getter for the kind of the Type.
    /// @return Returns the kind of the type as a TypeKind.
    TypeKind getKind() const { return _kind; }

    /// @brief Base contructor for all Types, it also initializes the TypeKind
    ///        for LLVM RTTI to dynamicaly define class.
    /// @param kind Defines the kind of the Type to initialize it dynamicaly
    ///             with LLVM RTTI
    TypeBase(TypeKind kind) : _kind(kind) { }

    /// @brief Polymorphic equality operator
    /// @param other The other TypeBase to compare with
    /// @return Returns true if the two types are equal, false otherwise
    bool operator==(TypeBase const &other) const;

    /// @brief Polymorphic hash function
    /// @return Returns the hash value of the type
    unsigned hash() const;
};

using Ty = TypeBase *;

} // namespace glu::types

#endif /* !GLU_AST_TYPES_TYPEBASE_HPP */
