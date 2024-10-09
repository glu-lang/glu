#ifndef GLU_AST_TYPES_TYPEBASE_HPP_
#define GLU_AST_TYPES_TYPEBASE_HPP_

#include "llvm/Support/Casting.h"
#include <string>

namespace glu::types {

/// @brief Base class for every Type definition. Contains the most basic
///        elements each types should at least have.
class TypeBase {

public:
    /// @brief Discriminator for LLVM-style RTTI (used in dyn_cast<> and similar
    /// operations)
    enum class TypeKind {
        /// Represents a structure type.
        /// Used to describe types defined as structures (e.g., structs in
        /// C/C++).
        StructTyKind,

        /// Represents an enum type.
        /// Used to describe types defined as enumerations (e.g., enum in
        /// C/C++).
        EnumTyKind,

        /// Represents an integer type.
        /// Includes various bit-width integer types (e.g., int32, int64).
        IntTyKind,

        /// Represents a floating-point type.
        /// Includes types such as float, double, etc.
        FloatTyKind,

        /// Represents a character type.
        /// Typically used for single character types (e.g., char in C/C++).
        CharTyKind,

        /// Represents a boolean type.
        /// Used for true/false values.
        BoolTyKind,

        /// Represents a function type.
        /// Describes a function signature, including return type and parameter
        /// types.
        FunctionTyKind,

        /// Represents an array type.
        /// Used to describe arrays of a fixed or dynamic length.
        ArrayTyKind,

        /// Represents a pointer type.
        /// Describes types that are pointers to other types (e.g., int*).
        PointerTyKind,

        /// Represents a type alias.
        /// Used to refer to types through aliases (e.g., typedef in C/C++).
        TypeAliasTyKind,
    };

    /// @brief Getter for the kind of the Type.
    /// @return Returns the size of the type as a TypeKind.
    inline TypeKind getKind() const
    {
        return _kind;
    }

    /// @brief Base contructor for all Types, it also initializes the TypeKind
    ///        for LLVM RTTI to dynamicaly define class.
    /// @param binding Defines whether the type is constant or variable.
    /// @param name Defines the name of the type as a String.
    /// @param size Defines the size of the type in Bytes.
    /// @param kind Defines the kind of the Type to initialize it dynamicaly
    ///             with LLVM RTTI
    TypeBase(std::string name, size_t size, TypeKind kind)
        : _kind(kind)
    {
    }

private:
    TypeKind const _kind;
};
}
#endif /* !GLU_AST_TYPES_TYPEBASE_HPP_ */
