#ifndef GLU_AST_EXPR_BINARY_OP_EXPR_HPP
#define GLU_AST_EXPR_BINARY_OP_EXPR_HPP

#include "RefExpr.hpp"

namespace glu::ast {

/// @brief Represents a binary operation expression in the AST (e.g., x + y, a *
/// b, value1 && value2)
class BinaryOpExpr : public ExprBase {
    ExprBase *_leftOperand;
    RefExpr *_op;
    ExprBase *_rightOperand;

public:
    /// @brief Constructs a BinaryOpExpr.
    /// @param loc The source location of the operator token
    /// @param leftOperand The left operand expression
    /// @param op The operator token
    /// @param rightOperand The right operand expression
    BinaryOpExpr(
        SourceLocation loc, ExprBase *leftOperand, RefExpr *op,
        ExprBase *rightOperand
    )
        : ExprBase(NodeKind::BinaryOpExprKind, loc)
        , _leftOperand(leftOperand)
        , _op(op)
        , _rightOperand(rightOperand)
    {
        assert(leftOperand && "Left operand cannot be null.");
        assert(op && "Operator cannot be null.");
        assert(rightOperand && "Right operand cannot be null.");
        leftOperand->setParent(this);
        op->setParent(this);
        rightOperand->setParent(this);
    }

    /// @brief Returns the left operand expression.
    /// @return The left operand expression
    ExprBase *getLeftOperand() const { return _leftOperand; }

    /// @brief Returns the operator token representing the binary operation.
    /// @return The operator token
    RefExpr *getOperator() const { return _op; }

    /// @brief Returns the right operand expression.
    /// @return The right operand expression
    ExprBase *getRightOperand() const { return _rightOperand; }

    /// @brief Sets the left operand expression.
    /// @param leftOperand The new left operand
    void setLeftOperand(ExprBase *leftOperand)
    {
        _leftOperand = leftOperand;
        if (leftOperand)
            leftOperand->setParent(this);
    }

    /// @brief Sets the right operand expression.
    /// @param rightOperand The new right operand
    void setRightOperand(ExprBase *rightOperand)
    {
        _rightOperand = rightOperand;
        if (rightOperand)
            rightOperand->setParent(this);
    }

    /// @brief Sets the operator expression.
    /// @param op The new operator
    void setOperator(RefExpr *op)
    {
        _op = op;
        if (op)
            op->setParent(this);
    }

    /// @brief Checks if the given AST node is a BinaryOpExpr.
    /// @param node The AST node to check
    /// @return True if the node is of type BinaryOpExpr, otherwise false
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::BinaryOpExprKind;
    }
};

}

#endif // GLU_AST_EXPR_BINARY_OP_EXPR_HPP
