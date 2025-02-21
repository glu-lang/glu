#ifndef GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
#define GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

class LiteralExpr : public ExprBase {
    glu::types::TypeBase *_type;

public:
    LiteralExpr(glu::types::TypeBase *type, SourceLocation loc)
        : ExprBase(NodeKind::LiteralExprKind, loc), _type(type) { };

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::LiteralExprKind;
    }

    glu::types::TypeBase *getType() { return _type; }
};

}

#endif // GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
