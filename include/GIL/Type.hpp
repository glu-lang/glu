#ifndef GLU_GIL_TYPE_HPP
#define GLU_GIL_TYPE_HPP

#include "Types/PointerTy.hpp"
#include "Types/TypeBase.hpp"

#include <llvm/ADT/DenseMapInfo.h>

namespace glu::gil {

/// @brief Represents a type in the GIL (Glu Intermediate Language).
/// For more information, see the documentation here:
/// https://glu-lang.org/gil/
class Type {
    /// @brief struct to store the fields of the Type class.
    ///        It has 10 bytes that are unused.
    struct Fields {
        uint64_t size : 48 = 0;
        uint64_t alignment : 5 = 0;
        uint64_t isConst : 1 = 0;
    } _fields;
    glu::types::TypeBase *_type = nullptr;

public:
    /// @brief Constructor for the Type class.
    /// @param size The size of the type in bytes.
    /// @param alignment The alignment of the type in bytes.
    /// @param isConst Whether the type is const or not.
    /// @param type A pointer to the type base.
    Type(
        size_t size, size_t alignment, bool isConst, glu::types::TypeBase *type
    )
        : _fields { size, alignment, isConst }, _type(type)
    {
        assert(_fields.size == size && "Size is bigger than 48 bits!");
        assert(
            _fields.alignment == alignment && "Alignment is larger than 5 bits!"
        );
    }

    Type() = default;

    /// @brief Getter for the size of the type.
    /// @return Returns the size of the type in bytes.
    unsigned getSize() const { return _fields.size; }

    /// @brief Getter for the alignment of the type.
    /// @return Returns the alignment of the type in bytes.
    unsigned getAlignment() const { return _fields.alignment; }

    /// @brief Getter for the constness of the type.
    /// @return Returns whether the type is const or not.
    bool isConst() const { return _fields.isConst; }

    /// @brief Getter for the type base.
    /// @return Returns a pointer to the type base.
    glu::types::TypeBase *getType() const { return _type; }

    /// @brief Dereference operator.
    /// @return A reference to the underlying TypeBase.
    glu::types::TypeBase &operator*() const { return *_type; }

    /// @brief Arrow operator for accessing members of the underlying TypeBase.
    /// @return A pointer to the underlying TypeBase.
    glu::types::TypeBase *operator->() const { return _type; }

    /// @brief Checks if two types are equal.

    /// @param other The other type to compare with.
    /// @return True if both types have the same size, alignment, constness, and
    /// base type.
    bool operator==(Type const &other) const { return _type == other._type; }
};

} // end namespace glu::gil

namespace llvm {

template <> struct DenseMapInfo<glu::gil::Type> {
    static inline glu::gil::Type getEmptyKey()
    {
        return glu::gil::Type(
            0, 0, false, DenseMapInfo<glu::types::TypeBase *>::getEmptyKey()
        );
    }

    static inline glu::gil::Type getTombstoneKey()
    {
        return glu::gil::Type(
            0, 0, false, DenseMapInfo<glu::types::TypeBase *>::getTombstoneKey()
        );
    }

    static unsigned getHashValue(glu::gil::Type const &type)
    {
        return DenseMapInfo<void *>::getHashValue(
            const_cast<glu::types::TypeBase *>(type.getType())
        );
    }

    static bool isEqual(glu::gil::Type const &lhs, glu::gil::Type const &rhs)
    {
        return lhs == rhs;
    }
};

} // end namespace llvm

#endif // GLU_GIL_TYPE_HPP
