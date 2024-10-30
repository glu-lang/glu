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
    PointerTy() : TypeBase(TypeKind::PointerTyKind) { }

    /// @brief Static method to check if a type is a PointerTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `PointerTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::PointerTyKind;
    }

    TypeBase* getType() {
        return _type;
    }

};
} // end namespace glu::types

#endif // GLU_AST_TYPES_POINTERTY_HPP
