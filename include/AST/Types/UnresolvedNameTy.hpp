#ifndef GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
#define GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP

#include "TypeBase.hpp"
#include <string>

namespace glu::types {

/// @brief UnresolvedNameTy is a class that represents an unresolved name type
/// in the AST.
class UnresolvedNameTy : public TypeBase {
    std::string const _name;

public:
    UnresolvedNameTy(std::string name)
        : TypeBase(TypeKind::UnresolvedNameTyKind), _name(std::move(name))
    {
    }

    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::UnresolvedNameTyKind;
    }
};

}

#endif // GLU_AST_TYPES_UNRESOLVEDNAMETY_HPP
