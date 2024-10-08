#ifndef GLU_AST_TYPES_TYPEBASE_HPP_
#define GLU_AST_TYPES_TYPEBASE_HPP_

#include "llvm/Support/Casting.h"
#include <string>

namespace glu::types {

/// @brief This enum represents the binding Type of a type, which corresponds to
///        whether this type is constant or variable.
enum BindingType {
    /// @brief The type is variable
    Var,
    /// @brief The type is constant
    Const
};

/// @brief Base class for every Type definition. Contains the most basic
///        elements each types should at least have.
class TypeBase {
protected:
    BindingType const _binding;
    std::string const _name;
    size_t const _size;

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

    TypeBase(BindingType binding, std::string name, size_t size, TypeKind kind)
        : _binding(binding)
        , _kind(kind)
        , _name(name)
        , _size(size)
    {
    }

    /// @brief Virtual destructor for polymorphisme
    virtual ~TypeBase() = default;

    /// @brief Virtual getter for the size of the Type.
    /// @return Returns the size of the type as a size_t.
    virtual inline size_t getSize() const
    {
        return _size;
    }

    /// @brief Virtual getter for the Binding of the Type.
    /// @return Returns the Binding of the Type as a BindingType.
    virtual inline BindingType getBinding() const
    {
        return _binding;
    }

    /// @brief Virtual getter for the Name of the Type.
    /// @return Returns the Name of the Type as a String.
    virtual inline std::string getName() const
    {
        return _name;
    }

private:
    TypeKind const _kind;
};

}
#endif /* !GLU_AST_TYPES_TYPEBASE_HPP_ */
