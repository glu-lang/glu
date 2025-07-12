#ifndef GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
#define GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP

#include "TypeBase.hpp"
#include <string>

namespace glu::types {

/// @brief UnresolvedNameTy is a class that represents an unresolved name type
/// in the AST.
class UnresolvedNameTy : public TypeBase {
    llvm::StringRef _name;

public:
    UnresolvedNameTy(llvm::StringRef name)
        : TypeBase(TypeKind::UnresolvedNameTyKind), _name(name)
    {
    }

    /// @brief Getter for the name of the unresolved type.
    /// @return The name of the unresolved type.
    llvm::StringRef getName() const { return _name; }
    void setName(llvm::StringRef name) { _name = name; }

    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::UnresolvedNameTyKind;
    }
};

}

#endif // GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
