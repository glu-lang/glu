#ifndef GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP
#define GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @brief Represents a ternary conditional expression in the AST (e.g., x ? y :
/// z).
class TernaryConditionalExpr : public ExprBase {
    ExprBase *_condition;
    ExprBase *_trueExpr;
    ExprBase *_falseExpr;

public:
    /// @brief Initializes the ternary conditional expression.
    /// @param loc The location of the expression
    /// @param condition The condition expression.
    /// @param trueExpr The expression to evaluate if the condition is true.
    /// @param falseExpr The expression to evaluate if the condition is false.
    TernaryConditionalExpr(
        SourceLocation loc, ExprBase *condition, ExprBase trueExpr,
        ExprBase falseExpr
    )
        : ExprBase(NodeKind::TernaryConditionalExprKind, loc)
        , _condition(condition)
        , _trueExpr(trueExpr)
        , _falseExpr(falseExpr)
    {
    }

    /// @brief Returns the condition expression.
    /// @return The condition expression.
    ExprBase *getCondition() const { return _condition; }

    /// @brief Returns the expression to evaluate if the condition is true.
    /// @return The expression to evaluate if the condition is true.
    ExprBase *getTrueExpr() const { return _trueExpr; }

    /// @brief Returns the expression to evaluate if the condition is false.
    /// @return The expression to evaluate if the condition is false.
    ExprBase *getFalseExpr() const { return _falseExpr; }

    /// @brief Checks if the node is a ternary conditional expression.
    /// @param node The node to check.
    /// @return True if the node is a ternary conditional expression.
    static bool classof(ASTNode const *node)
    {
        return node->kind() == ASTNodeKind::TernaryConditionalExpr;
    }
};

}

#endif // GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP
