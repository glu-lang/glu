#ifndef GLU_AST_TYPES_DYNAMIC_ARRAYTY_HPP_
#define GLU_AST_TYPES_DYNAMIC_ARRAYTY_HPP_

#include "Types/TypeBase.hpp"

namespace glu::types {

/// @brief Represents an array type.
class DynamicArrayTy : public TypeBase {
    TypeBase *_dataType;

public:
    /// @brief Constructor for the DynamicArrayTy class.
    /// @param dataType The kind of the data that the array will hold.
    DynamicArrayTy(TypeBase *dataType)
        : TypeBase(TypeKind::DynamicArrayTyKind), _dataType(dataType) { };

    /// @brief Getter for the kind of the data that the dynamic array will hold.
    /// @return Returns the kind of the data that the dynamic array will hold.
    TypeBase *getDataKind() const { return _dataType; }

    /// @brief Static method to check if a type is a DynamicArrayTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::DynamicArrayTyKind;
    }
};
}
#endif /* !GLU_AST_TYPES_DYNAMIC_ARRAYTY_HPP_ */
