#ifndef GLU_GIL_MEMBER_HPP
#define GLU_GIL_MEMBER_HPP

#include "AST/Types.hpp"
#include "Type.hpp"
#include "Value.hpp"

namespace glu::gil {

/// @class Member
/// @brief Represents a member (field) of a structure or an enum in the Glu
/// language.
///
/// This class encapsulates the necessary information to represent and access
/// a struct member in the GLU GIL (Generic Intermediate Language).
class Member {
    Type _type; ///< Pointer to the type contained in this member
    llvm::StringRef _name; ///< The name of the member

public:
    /// @brief Constructs a Member object.
    ///
    /// @param field The field metadata information
    /// @param value Pointer to the value contained in this member
    Member(Value parent, Type type, llvm::StringRef name)
        : _type(type), _name(name)
    {
    }

    /// @brief Gets the field metadata.
    ///
    /// @return The field metadata information.
    llvm::StringRef getName() const { return _name; }

    /// @brief Gets the value contained in this member.
    ///
    /// @return Pointer to the value contained in this member.
    Type getType() const { return _type; }
};

}

#endif // GLU_GIL_MEMBER_HPP
