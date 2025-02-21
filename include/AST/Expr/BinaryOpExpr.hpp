#ifndef GLU_AST_EXPR_BINARY_OP_EXPR_HPP
#define GLU_AST_EXPR_BINARY_OP_EXPR_HPP

#include "ASTNode.hpp"

namespace glu::ast {

class BinaryOpExpr : public ExprBase {
    ;

public:
    BinaryOpExpr(SourceLocation loc)
        : ExprBase(NodeKind::BinaryOpExprKind, loc) { };

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::BinaryOpExprKind;
    }
};
} // namespace glu::ast

#endif // GLU_AST_EXPR_BINARY_OP_EXPR_HPP
