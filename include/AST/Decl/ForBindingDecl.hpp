#ifndef GLU_AST_DECL_FORBINDINGDECL_HPP
#define GLU_AST_DECL_FORBINDINGDECL_HPP

#include "VarLetDecl.hpp"

namespace glu::ast {

class ForBindingDecl : public VarLetDecl {

public:
    ForBindingDecl(
        SourceLocation location, std::string name, ExprBase *init,
        glu::types::TypeBase *type
    )
        : VarLetDecl(NodeKind::ForBindingDeclKind, location, name, type, init)
    {
    }
};

} // namespace glu::ast

#endif // GLU_AST_DECL_FORBINDINGDECL_HPP
