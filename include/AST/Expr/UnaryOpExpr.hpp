#ifndef GLU_AST_EXPR_UNARY_OP_EXPR_HPP
#define GLU_AST_EXPR_UNARY_OP_EXPR_HPP

#include "ASTNodeMacros.hpp"
#include "RefExpr.hpp"

namespace glu::ast {

/// @brief Represents a unary operation expression in the AST (e.g., -x, ~0,
/// val.*).
class UnaryOpExpr : public ExprBase {

    GLU_AST_GEN_CHILD(UnaryOpExpr, ExprBase *, _value, Operand)
    GLU_AST_GEN_CHILD(UnaryOpExpr, RefExpr *, _op, Operator)

public:
    /// @brief Constructs a UnaryOpExpr.
    /// @param loc The source location of the operator token
    /// @param value The operand of the unary operation
    /// @param op The operator token
    UnaryOpExpr(SourceLocation loc, ExprBase *value, RefExpr *op)
        : ExprBase(NodeKind::UnaryOpExprKind, loc)
    {
        initOperand(value);
        initOperator(op);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::UnaryOpExprKind;
    }
};

}

#endif // GLU_AST_EXPR_UNARY_OP_EXPR_HPP
