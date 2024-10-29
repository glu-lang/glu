#ifndef GLU_AST_TYPES_ARRAYTY_HPP_
#define GLU_AST_TYPES_ARRAYTY_HPP_

#include "Types/TypeBase.hpp"
#include <optional>

namespace glu::types {

/// @brief Represents an array type.
class ArrayTy : public TypeBase {
    TypeKind const _dataKind;
    enum ArrayMode { Dynamic, Static } _mode;
    /// @brief The size of the array if the mode is static.
    std::optional<std::size_t> _size;

public:
    /// @brief Constructor for the ArrayTy class.
    /// @param dataKind The kind of the data that the array will hold.
    /// @param mode The mode of the array (dynamic or static),
    ///             default is dynamic.
    /// @param size The size of the array if the mode is static.
    ArrayTy(
        TypeKind dataKind, ArrayMode mode = ArrayMode::Dynamic,
        std::size_t size = 0
    )
        : TypeBase(TypeKind::ArrayTyKind), _dataKind(dataKind), _mode(mode)
    {
        if (mode == ArrayMode::Static)
            _size = size;
        else
            _size = std::nullopt;
    };

    /// @brief Getter for the kind of the data that the array will hold.
    /// @return Returns the kind of the data that the array will hold.
    TypeKind getDataKind() const { return _dataKind; }

    /// @brief Getter for the mode of the array.
    /// @return Returns the mode of the array.
    ArrayMode getMode() const { return _mode; }

    /// @brief Getter for the size of the array if the array if a static.
    /// @return Returns the size of the array or nullopt if the array is
    /// dynamic.
    std::optional<std::size_t> getSize() const { return _size; }

    /// @brief Static method to check if a type is an ArrayTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::ArrayTyKind;
    }
};
}
#endif /* !GLU_AST_TYPES_ARRAYTY_HPP_ */
