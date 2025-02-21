#ifndef GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
#define GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP

#include "ASTNode.hpp"

namespace glu::ast {

class LiteralExpr : public ExprBase {
public:
    LiteralExpr(SourceLocation loc)
        : ExprBase(NodeKind::LiteralExprKind, loc) { };

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::LiteralExprKind;
    }
};

}

#endif // GLU_AST_EXPR_LITERAL_EXPR_KIND_HPP
