#ifndef GLU_AST_EXPR_POINTER_DEREF_EXPR_KIND_HPP
#define GLU_AST_EXPR_POINTER_DEREF_EXPR_KIND_HPP

#include "ASTNode.hpp"

namespace glu::ast {

class PointerDerefExpr : public ExprBase {
    ExprBase *_value;

public:
    PointerDerefExpr(SourceLocation loc, ExprBase *value)
        : ExprBase(NodeKind::PointerDerefExprKind, loc), _value(value)
    {
    }

    ExprBase *getPointerExpr() const { return _value; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::PointerDerefExprKind;
    }
};

}

#endif // GLU_AST_EXPR_POINTER_DEREF_EXPR_KIND_HPP
