#ifndef GLU_AST_EXPR_BINARY_OP_EXPR_HPP
#define GLU_AST_EXPR_BINARY_OP_EXPR_HPP

#include "ASTNodeMacros.hpp"
#include "RefExpr.hpp"

namespace glu::ast {

/// @brief Represents a binary operation expression in the AST (e.g., x + y, a *
/// b, value1 && value2)
class BinaryOpExpr : public ExprBase {

    GLU_AST_GEN_CHILD(BinaryOpExpr, ExprBase *, _leftOperand, LeftOperand)
    GLU_AST_GEN_CHILD(BinaryOpExpr, RefExpr *, _op, Operator)
    GLU_AST_GEN_CHILD(BinaryOpExpr, ExprBase *, _rightOperand, RightOperand)

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
    {
        initLeftOperand(leftOperand);
        initOperator(op);
        initRightOperand(rightOperand);
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
