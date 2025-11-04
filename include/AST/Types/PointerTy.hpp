#ifndef GLU_AST_TYPES_POINTERTY_HPP
#define GLU_AST_TYPES_POINTERTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

enum class PointerKind {
    Shared,
    Unique,
    Raw
};

/// @brief Pointer is a class that represents the pointer type in the AST.
class PointerTy : public TypeBase {
private:
    TypeBase * const _pointee;
    PointerKind _kind;

public:
    /// @brief Constructor for the Pointer class.
    /// @param pointee The type this pointer points to.
    PointerTy(TypeBase *pointee, PointerKind kind = PointerKind::Raw)
        : TypeBase(TypeKind::PointerTyKind), _pointee(pointee), _kind(kind)
    {
    }

    /// @brief Static method to check if a type is a PointerTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `PointerTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::PointerTyKind;
    }

    /// @brief Getter for the pointee type.
    /// @return Returns the type this pointer points to.
    TypeBase *getPointee() const { return _pointee; }

    /// @brief Getter for the pointer kind.
    /// @return Returns the kind of the pointer (Shared, Unique, Raw).
    PointerKind getPointerKind() const { return _kind; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_POINTERTY_HPP
