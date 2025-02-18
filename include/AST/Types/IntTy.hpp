#ifndef GLU_AST_TYPES_INTTY_HPP
#define GLU_AST_TYPES_INTTY_HPP

#include "TypeBase.hpp"

#include <llvm/ADT/Hashing.h>

namespace glu::types {

/// @brief IntTy is a class that represents integer types, signed or unsigned,
/// of any bit width.
class IntTy : public TypeBase {
public:
    enum Signedness { Unsigned, Signed };

private:
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

    /// @brief Method to hash the IntTy.
    /// @return Returns the hash of the IntTy.
    std::size_t hash() const override
    {
        return llvm::hash_combine(getKind(), _signedness, _bitWidth);
    }

    /// @brief Method to compare two IntTy.
    /// @param other The other IntTy to compare.
    /// @return Returns `true` if the two IntTy are equal, `false` otherwise.
    bool operator==(TypeBase const &other) const override
    {
        if (auto *otherInt = llvm::dyn_cast<IntTy>(&other)) {
            return _signedness == otherInt->_signedness
                && _bitWidth == otherInt->_bitWidth;
        }
        return false;
    }
};

} // end namespace glu::types

#endif // GLU_AST_TYPES_INTTY_HPP
