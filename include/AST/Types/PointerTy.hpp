#ifndef GLU_AST_TYPES_POINTERTY_HPP
#define GLU_AST_TYPES_POINTERTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief Pointer is a class that represents the pointer type in the AST.
class PointerTy : public TypeBase {
private:
    TypeBase * const _pointee;

public:
    /// @brief Constructor for the Pointer class.
    /// @param pointee The type this pointer points to.
    PointerTy(TypeBase *pointee)
        : TypeBase(TypeKind::PointerTyKind), _pointee(pointee)
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
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_POINTERTY_HPP
