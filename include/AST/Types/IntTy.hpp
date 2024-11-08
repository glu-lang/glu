#ifndef GLU_AST_TYPES_INTTY_HPP
#define GLU_AST_TYPES_INTTY_HPP

#include "TypeBase.hpp"

namespace glu::types {

enum Signedness { Unsigned, Signed };

/// @brief IntTy is a class that represents integer types, signed or unsigned,
/// of any bit width.
class IntTy : public TypeBase {
    Signedness _signedness;
    unsigned _bitWidth;

public:
    /// @brief Constructor for the IntTy class.
    IntTy(Signedness signedness, unsigned bitWidth)
        : TypeBase(TypeKind::IntTyKind)
        , _signedness(signedness)
        , _bitWidth(bitWidth)
    {
        assert(bitWidth > 0 && "Bit width must be greater than 0");
    }

    Signedness getSignedness() const { return _signedness; }
    bool isSigned() const { return _signedness == Signed; }
    bool isUnsigned() const { return _signedness == Unsigned; }

    unsigned getBitWidth() const { return _bitWidth; }

    /// @brief Static method to check if a type is an IntTy.
    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::IntTyKind;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_INTTY_HPP
