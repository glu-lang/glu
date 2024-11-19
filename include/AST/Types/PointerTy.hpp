#ifndef GLU_AST_TYPES_POINTERTY_HPP
#define GLU_AST_TYPES_POINTERTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

/// @brief Pointer is a class that represents the pointer type in the AST.
class PointerTy : public TypeBase {
private:
    TypeBase * const _type;

public:
    /// @brief Constructor for the Pointer class.
    /// @param type The type this pointer points to.
    PointerTy(TypeBase *type) : TypeBase(TypeKind::PointerTyKind), _type(type)
    {
    }

    /// @brief Static method to check if a type is a PointerTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `PointerTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::PointerTyKind;
    }

    /// @brief Getter for the type this pointer points to.
    /// @return Returns the type this pointer points to.
    TypeBase *getType() const { return _type; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_POINTERTY_HPP
