#ifndef GLU_AST_STMT_ASSIGNSTMT_HPP
#define GLU_AST_STMT_ASSIGNSTMT_HPP

#include "ASTNode.hpp"
#include "Basic/Tokens.hpp"

namespace glu::ast {

/// @class AssignStmt
/// @brief Represents an assignment statement in the AST.
///
/// This class stores the expressions for the left-hand side and the right-hand side,
/// and the assignment operator token is always "=".
class AssignStmt : public StmtBase {
    ExprBase *_exprLeft;
    Token _operator;
    ExprBase *_exprRight;

public:
    /// @brief Constructor for the AssignStmt class.
    /// @param location The source location of the assignment statement.
    /// @param exprLeft The left-hand side expression.
    /// @param _operator The assignment operator (e.g., '=', '+=', '-=', etc.).
    /// @param exprRight The right-hand side expression.
    AssignStmt(
        SourceLocation location, ExprBase *_exprLeft, Token _operator,
        ExprBase *_exprRight
    )
        : StmtBase(NodeKind::AssignStmtKind, location)
        , _exprLeft(_exprLeft)
        , _operator(_operator)
        , _exprRight(_exprRight)
    {
        assert(_exprLeft && "Left-hand side expression cannot be null.");
        assert(_exprRight && "Right-hand side expression cannot be null.");
        _exprLeft->setParent(this);
        _exprRight->setParent(this);
    }

    /// @brief Returns the left-hand side expression.
    ExprBase* getExprLeft() const { return _exprLeft; }

    /// @brief Returns the assignment operator token (always "=").
    Token getOperator() const { return _operator; }

    /// @brief Returns the right-hand side expression.
    ExprBase* getExprRight() const { return _exprRight; }

    /// @brief Check if the given node is an assignment statement.
    /// @param node The node to check.
    /// @return True if the node is an assignment statement, false otherwise.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::AssignStmtKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_STMT_ASSIGNSTMT_HPP
