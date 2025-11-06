#ifndef GLU_AST_TYPES_TEMPLATEPARAMTY_HPP
#define GLU_AST_TYPES_TEMPLATEPARAMTY_HPP

#include "TypeBase.hpp"

namespace glu::ast {
class TemplateParameterDecl;
}

namespace glu::types {

/// Type representing a template parameter declaration in the AST.
class TemplateParamTy final : public TypeBase {
    glu::ast::TemplateParameterDecl *_decl;

public:
    explicit TemplateParamTy(glu::ast::TemplateParameterDecl *decl)
        : TypeBase(TypeKind::TemplateParamTyKind), _decl(decl)
    {
    }

    glu::ast::TemplateParameterDecl *getDecl() const { return _decl; }

    static bool classof(TypeBase const *type)
    {
        return type->getKind() == TypeKind::TemplateParamTyKind;
    }
};

} // namespace glu::types

#endif // GLU_AST_TYPES_TEMPLATEPARAMTY_HPP
