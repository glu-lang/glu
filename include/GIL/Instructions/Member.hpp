#ifndef GLU_GIL_MEMBER_HPP
#define GLU_GIL_MEMBER_HPP

#include "AST/Types.hpp"
#include "Type.hpp"
#include "Value.hpp"

#include <llvm/ADT/DenseMapInfo.h>

namespace glu::gil {

/// @class Member
/// @brief Represents a member of a type, such as a struct field or an
///        enum variant
class Member {
    std::string _name; ///< The name of the member.
    Type _type; ///< The type of the member.
    Type _parent; ///< The parent type that contains this member (must represent
                  ///< a StructTy or EnumTy).

public:
    /// @brief Constructs a Member with the given name, type, and parent type.
    /// @param name The name of the member.
    /// @param type The type of the member.
    /// @param parent A pointer to the parent type that contains this member.
    Member(llvm::StringRef name, Type type, Type parent)
        : _name(name), _type(type), _parent(parent)
    {
    }

    /// @brief Gets the name of this member.
    /// @return The name of the member as a string.
    std::string getName() const { return _name; }

    /// @brief Gets the type of this member.
    /// @return The type of the member.
    Type getType() const { return _type; }

    /// @brief Gets the parent type that contains this member.
    /// @return A pointer to the parent type.
    Type getParent() const { return _parent; }

    /// @brief Checks if two members are equal.
    /// @param other The other member to compare with.
    /// @return True if both members have the same name, parent, and type.
    bool operator==(Member const &other) const
    {
        return _name == other._name && _parent == other._parent
            && _type == other._type;
    }

    /// @brief Checks if two members are different.
    /// @param other The other member to compare with.
    /// @return True if the members are not equal.
    bool operator!=(Member const &other) const { return !(*this == other); }

    /// @brief Gets the empty key for DenseMap.
    /// @return A default-constructed Member representing an empty entry.
    static Member getEmptyKey()
    {
        return Member(
            llvm::DenseMapInfo<llvm::StringRef>::getEmptyKey(), Type(), Type()
        );
    }

    /// @brief Gets the tombstone key for DenseMap.
    /// Used to mark deleted entries in the hash table.
    /// @return A special Member instance representing a deleted entry.
    static Member getTombstoneKey()
    {
        return Member(
            llvm::DenseMapInfo<llvm::StringRef>::getTombstoneKey(),
            Type(0, 0, false, reinterpret_cast<glu::types::TypeBase *>(-1)),
            Type(0, 0, false, reinterpret_cast<glu::types::TypeBase *>(-1))
        );
    }
};

}
namespace llvm {

// support for Member keys in DenseMap
template <> struct DenseMapInfo<glu::gil::Member> {
    static inline glu::gil::Member getEmptyKey()
    {
        return glu::gil::Member::getEmptyKey();
    }

    static inline glu::gil::Member getTombstoneKey()
    {
        return glu::gil::Member::getTombstoneKey();
    }

    static unsigned getHashValue(glu::gil::Member const &member)
    {
        return DenseMapInfo<std::pair<glu::gil::Type, llvm::StringRef>>::
            getHashValue(std::make_pair(member.getParent(), member.getName()));
    }

    static bool
    isEqual(glu::gil::Member const &lhs, glu::gil::Member const &rhs)
    {
        return lhs == rhs;
    }
};
} // end namespace llvm

#endif // GLU_GIL_MEMBER_HPP
