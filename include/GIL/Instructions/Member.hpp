#ifndef GLU_GIL_MEMBER_HPP
#define GLU_GIL_MEMBER_HPP

#include "AST/Types.hpp"
#include "Type.hpp"

namespace glu::gil {

class Value;

/// @class Member
/// @brief Represents a member (field) of a structure in the Glu language.
///
/// This class encapsulates the necessary information to represent and access
/// a struct member in the GLU GIL (Generic Intermediate Language).
class Member {
    Value *_parentStruct; ///< The value of the parent structure
    Value *_value; ///< Pointer to the value contained in this member
    llvm::StringRef _name; ///< The name of the member

public:
    /// @brief Constructs a Member object.
    ///
    /// @param parentStruct The type of the parent structure
    /// @param field The field metadata information
    /// @param value Pointer to the value contained in this member
    Member(Value *parentStruct, Value *value, llvm::StringRef name)
        : _parentStruct(parentStruct), _value(value), _name(name)
    {
    }

    /// @brief Gets the parent structure type.
    ///
    /// @return The type of the parent structure.
    Value *getParentStruct() const { return _parentStruct; }

    /// @brief Gets the field metadata.
    ///
    /// @return The field metadata information.
    llvm::StringRef getName() const { return _name; }

    /// @brief Gets the value contained in this member.
    ///
    /// @return Pointer to the value contained in this member.
    Value *getValue() const { return _value; }
};

}

#endif // GLU_GIL_MEMBER_HPP
