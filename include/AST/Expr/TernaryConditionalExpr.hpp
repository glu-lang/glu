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
        SourceLocation loc, ExprBase *condition, ExprBase *trueExpr,
        ExprBase *falseExpr
    )
        : ExprBase(NodeKind::TernaryConditionalExprKind, loc)
        , _condition(condition)
        , _trueExpr(trueExpr)
        , _falseExpr(falseExpr)
    {
        assert(condition && "Condition cannot be null.");
        assert(trueExpr && "True expression cannot be null.");
        assert(falseExpr && "False expression cannot be null.");
        condition->setParent(this);
        trueExpr->setParent(this);
        falseExpr->setParent(this);
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

    void setCondition(ExprBase *condition)
    {
        if (_condition != nullptr) {
            _condition->setParent(nullptr);
        }
        _condition = condition;
        if (condition)
            condition->setParent(this);
    }
    void setTrueExpr(ExprBase *trueExpr)
    {
        if (_trueExpr != nullptr) {
            _trueExpr->setParent(nullptr);
        }
        _trueExpr = trueExpr;
        if (trueExpr)
            trueExpr->setParent(this);
    }
    void setFalseExpr(ExprBase *falseExpr)
    {
        if (_falseExpr != nullptr) {
            _falseExpr->setParent(nullptr);
        }
        _falseExpr = falseExpr;
        if (falseExpr)
            falseExpr->setParent(this);
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
