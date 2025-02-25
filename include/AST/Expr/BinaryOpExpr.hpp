#ifndef GLU_AST_EXPR_BINARY_OP_EXPR_HPP
#define GLU_AST_EXPR_BINARY_OP_EXPR_HPP

#include "ASTNode.hpp"
#include "Basic/Tokens.hpp"

namespace glu::ast {

/// @brief Represents a binary operation expression in the AST (e.g., x + y, a *
/// b, value1 && value2)
class BinaryOpExpr : public ExprBase {
    ExprBase *_leftOperand;
    Token _operator;
    ExprBase *_rightOperand;

public:
    /// @brief Constructs a BinaryOpExpr.
    /// @param loc The source location of the operator token
    /// @param leftOperand The left operand expression
    /// @param operator The operator token
    /// @param rightOp The right operand expression
    BinaryOpExpr(
        SourceLocation loc, ExprBase *leftOperand, Token operator,
        ExprBase * rightOp
    )
        : ExprBase(NodeKind::BinaryOpExprKind, loc)
        , _leftOperand(leftOperand)
        , _operator(operator)
        , _rightOperand(rightOperand)
    {
    }

    /// @brief Returns the left operand expression.
    /// @return The left operand expression
    ExprBase *getLeftOperand() const { return _leftOperand; }

    /// @brief Returns the operator token representing the binary operation.
    /// @return The operator token
    Token getOperator() const { return _operator; }

    /// @brief Returns the right operand expression.
    /// @return The right operand expression
    ExprBase *getRightOperand() const { return _rightOperand; }

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
