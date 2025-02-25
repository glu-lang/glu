#ifndef GLU_AST_EXPR_BINARY_OP_EXPR_HPP
#define GLU_AST_EXPR_BINARY_OP_EXPR_HPP

#include "ASTNode.hpp"
#include "Basic/Tokens.hpp"

namespace glu::ast {

/// @brief Represents a unary operation expression in the AST (e.g., -x, ~0,
/// val.*).
class BinaryOpExpr : public ExprBase {
    ExprBase *_leftOperand;
    Token _operator;
    ExprBase *_rightOperand;

public:
    /// @brief Constructs a BinaryOpExpr.
    /// @param loc The source location of the operator token
    /// @param value The operand of the unary operation
    /// @param op The operator token
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

    /// @brief Returns the operand of the unary operation.
    ExprBase *getLeftOperand() const { return _leftOperand; }

    /// @brief Returns the operator token, whose kind is the unary operator
    /// being applied.
    Token getOperator() const { return _operator; }

    /// @brief Returns the operand of the unary operation.
    ExprBase *getRightOperand() const { return _rightOperand; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::BinaryOpExprKind;
    }
};

}

#endif // GLU_AST_EXPR_BINARY_OP_EXPR_HPP
