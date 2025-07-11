#ifndef GLU_AST_TYPES_STATIC_ARRAYTY_HPP_
#define GLU_AST_TYPES_STATIC_ARRAYTY_HPP_

#include "Types/TypeBase.hpp"

namespace glu::types {

/// @brief Represents an array type.
class StaticArrayTy : public TypeBase {
    TypeBase *_dataType;
    std::size_t _size;

public:
    /// @brief Constructor for the StaticArrayTy class.
    /// @param dataKind The kind of the data that the array will hold.
    /// @param size The size of the array if the mode is static.
    StaticArrayTy(TypeBase *dataType, std::size_t size)
        : TypeBase(TypeKind::StaticArrayTyKind)
        , _dataType(dataType)
        , _size(size) { };

    /// @brief Getter for the type of the data that the static array will hold.
    /// @return Returns the type of the data that the static array will hold.
    TypeBase *getDataType() const { return _dataType; }

    /// @brief Getter for the size of the static array.
    /// @return Returns the size of the static array.
    std::size_t getSize() const { return _size; }

    /// @brief Static method to check if a type is a StaticArrayTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::StaticArrayTyKind;
    }
};
}
#endif /* !GLU_AST_TYPES_STATIC_ARRAYTY_HPP_ */
