#ifndef GLU_AST_EXPR_CAST_EXPR_EXPR_KIND_HPP
#define GLU_AST_EXPR_CAST_EXPR_EXPR_KIND_HPP

#include "ASTNode.hpp"
#include "Types.hpp"

namespace glu::ast {

class CastExpr : public ExprBase {
    ExprBase *_value;
    glu::types::TypeBase *_destType;

public:
    CastExpr(
        SourceLocation loc, ExprBase *value, glu::types::TypeBase *destType
    )
        : ExprBase(NodeKind::CastExprKind, loc)
        , _value(value)
        , _destType(destType)
    {
    }

    ExprBase *getCastedExpr() const { return _value; }

    glu::types::TypeBase *getDestType() const { return _destType; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::CastExprKind;
    }
};

}

#endif // GLU_AST_EXPR_CAST_EXPR_EXPR_KIND_HPP
