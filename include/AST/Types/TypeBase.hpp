#ifndef GLU_AST_TYPES_TYPEBASE_HPP_
#define GLU_AST_TYPES_TYPEBASE_HPP_

#include <llvm/Support/Casting.h>
#include <string>

namespace glu::types {

/// @brief Base class for every Type definition. Contains the most basic
///        elements each types should at least have.
class TypeBase {

public:
    /// @brief Discriminator for LLVM-style RTTI (used in dyn_cast<> and similar
    /// operations)
    enum class TypeKind {
#define TYPE(NAME) NAME##Kind,
#include "Types/TypeKind.def"
#undef TYPE
    };

    /// @brief Getter for the kind of the Type.
    /// @return Returns the size of the type as a TypeKind.
    TypeKind getKind() const
    {
        return _kind;
    }

    /// @brief Base contructor for all Types, it also initializes the TypeKind
    ///        for LLVM RTTI to dynamicaly define class.
    /// @param kind Defines the kind of the Type to initialize it dynamicaly
    ///             with LLVM RTTI
    TypeBase(TypeKind kind)
        : _kind(kind)
    {
    }

private:
    TypeKind const _kind;
};
}
#endif /* !GLU_AST_TYPES_TYPEBASE_HPP_ */
