#ifndef GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP
#define GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

namespace glu::ast {

/// @brief Represents a ternary conditional expression in the AST (e.g., x ? y :
/// z).
class TernaryConditionalExpr : public ExprBase {

    GLU_AST_GEN_CHILD(TernaryConditionalExpr, ExprBase *, _condition, Condition)
    GLU_AST_GEN_CHILD(TernaryConditionalExpr, ExprBase *, _trueExpr, TrueExpr)
    GLU_AST_GEN_CHILD(TernaryConditionalExpr, ExprBase *, _falseExpr, FalseExpr)

public:
    /// @brief Initializes the ternary conditional expression.
    /// @param loc The location of the expression
    /// @param condition The condition expression.
    /// @param trueExpr The expression to evaluate if the condition is true.
    /// @param falseExpr The expression to evaluate if the condition is false.
    TernaryConditionalExpr(
        SourceLocation loc, ExprBase *condition, ExprBase *trueExpr,
        ExprBase *falseExpr
    )
        : ExprBase(NodeKind::TernaryConditionalExprKind, loc)
    {
        initCondition(condition);
        initTrueExpr(trueExpr);
        initFalseExpr(falseExpr);
    }

    /// @brief Checks if the node is a ternary conditional expression.
    /// @param node The node to check.
    /// @return True if the node is a ternary conditional expression.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::TernaryConditionalExprKind;
    }
};

}

#endif // GLU_AST_EXPR_TERNARY_CONDITIONAL_EXPR_HPP
