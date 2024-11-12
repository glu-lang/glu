#ifndef GLU_GIL_TYPE_HPP
#define GLU_GIL_TYPE_HPP

#include "TypeBase.hpp"

namespace glu::gil {

/// @brief Represents a type in the GIL (Glu Intermediate Language).
/// For more information, see the documentation here:
/// https://glu-lang.org/gil/
class Type {
    struct Fields {
        unsigned char size : 48;
        unsigned char alignment : 5;
        unsigned char isConst : 1;
    } _fields;
    glu::types::TypeBase *_type;

public:
    /// @brief Constructor for the Type class.
    /// @param size The size of the type in bytes.
    /// @param alignment The alignment of the type in bytes.
    /// @param isConst Whether the type is const or not.
    /// @param type A pointer to the type base.
    Type(
        unsigned size, unsigned alignment, bool isConst,
        glu::types::TypeBase *type
    )
        : _type(type)
    {
        _fields.size = size;
        _fields.alignment = alignment;
        _fields.isConst = isConst;
    }

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
};

} // end namespace glu::gil

#endif // GLU_GIL_TYPE_HPP
