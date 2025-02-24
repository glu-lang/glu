#ifndef GLU_AST_TYPES_FLOATTY_HPP
#define GLU_AST_TYPES_FLOATTY_HPP

#include "TypeBase.hpp"

namespace glu::types {
class FloatTy : public TypeBase {
    unsigned _bitWidth;

public:
    static constexpr unsigned HALF = 16;
    static constexpr unsigned FLOAT = 32;
    static constexpr unsigned DOUBLE = 64;
    static constexpr unsigned INTEL_LONG_DOUBLE = 80;

    /// @brief Constructor for the FloatTy class.
    FloatTy(unsigned bitWidth)
        : TypeBase(TypeKind::FloatTyKind), _bitWidth(bitWidth)
    {
        assert(bitWidth > 0 && "Bit width must be greater than 0");
    }

    unsigned getBitWidth() const { return _bitWidth; }

    /// @brief Static method to check if a type is a FloatTy.
    /// @param type The type to check.
    /// @return Returns `true` if the type is a `FloatTy`, `false` otherwise.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::FloatTyKind;
    }

    bool isHalf() const { return _bitWidth == HALF; }

    bool isFloat() const { return _bitWidth == FLOAT; }

    bool isDouble() const { return _bitWidth == DOUBLE; }

    bool isIntelLongDouble() const { return _bitWidth == INTEL_LONG_DOUBLE; }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_FLOATTY_HPP
