#ifndef GLU_AST_EXPR_UNARY_OP_EXPR_HPP
#define GLU_AST_EXPR_UNARY_OP_EXPR_HPP

#include "RefExpr.hpp"

namespace glu::ast {

/// @brief Represents a unary operation expression in the AST (e.g., -x, ~0,
/// val.*).
class UnaryOpExpr : public ExprBase {
    ExprBase *_value;
    RefExpr *_op;

public:
    /// @brief Constructs a UnaryOpExpr.
    /// @param loc The source location of the operator token
    /// @param value The operand of the unary operation
    /// @param op The operator token
    UnaryOpExpr(SourceLocation loc, ExprBase *value, RefExpr *op)
        : ExprBase(NodeKind::UnaryOpExprKind, loc), _value(value), _op(op)
    {
        assert(value && "Value cannot be null.");
        assert(op && "Operator cannot be null.");
        value->setParent(this);
        op->setParent(this);
    }

    /// @brief Returns the operand of the unary operation.
    ExprBase *getOperand() const { return _value; }

    /// @brief Returns the operator token, whose kind is the unary operator
    /// being applied.
    RefExpr *getOperator() const { return _op; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::UnaryOpExprKind;
    }
};

}

#endif // GLU_AST_EXPR_UNARY_OP_EXPR_HPP
