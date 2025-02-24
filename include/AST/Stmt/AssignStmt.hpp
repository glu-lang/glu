#ifndef GLU_AST_STMT_ASSIGNSTMT_HPP
#define GLU_AST_STMT_ASSIGNSTMT_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>

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
    /// @param exprRight The right-hand side expression.
    AssignStmt(SourceLocation location, ExprBase *_exprLeft, ExprBase *_exprRight)
        : StmtBase(NodeKind::AssignStmtKind, location)
        , _exprLeft(_exprLeft)
        , _operator(Token(TokenKind::Equal, "="))
        , _exprRight(_exprRight)
    {
    }

    /// @brief Returns the left-hand side expression.
    ExprBase* getExprLeft() const { return _exprLeft; }

    /// @brief Returns the assignment operator token (always "=").
    const Token& getOperator() const { return _operator; }

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
