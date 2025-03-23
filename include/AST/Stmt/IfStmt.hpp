#ifndef GLU_AST_STMT_IFSTMT_HPP
#define GLU_AST_STMT_IFSTMT_HPP

#include "ASTNode.hpp"

#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class IfStmt
/// @brief Represents a if statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a if
/// statement.
class IfStmt : public StmtBase {
    /// @brief The condition of the if statement.
    ExprBase *_condition;
    /// @brief The body of the if statement.
    CompoundStmt *_body;
    /// @brief The else branch of the if statement, or nullptr if there is none.
    CompoundStmt *_else;

public:
    /// @brief Constructor for the IfStmt class.
    /// @param location The source location of the if keyword.
    /// @param condition The condition of the if statement.
    /// @param body The body of the if statement.
    /// @param elseBranch The else branch of the if statement, or nullptr if
    /// there is none.
    IfStmt(
        SourceLocation location, ExprBase *condition, CompoundStmt *body,
        CompoundStmt *elseBranch = nullptr
    )
        : StmtBase(NodeKind::IfStmtKind, location)
        , _condition(condition)
        , _body(body)
        , _else(elseBranch)
    {
        assert(_condition && "Condition cannot be null.");
        assert(_body && "Body cannot be null.");
        condition->setParent(this);
        body->setParent(this);
        if (elseBranch)
            elseBranch->setParent(this);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::IfStmtKind;
    }

    /// @brief Get the condition of the if statement.
    /// @return The condition of the if statement.
    ExprBase *getCondition() { return _condition; }

    /// @brief Get the body of the if statement.
    /// @return The body of the if statement.
    CompoundStmt *getBody() { return _body; }

    /// @brief Get the else branch of the if statement, or nullptr if there is
    /// none.
    /// @return The else branch of the if statement.
    CompoundStmt *getElse() { return _else; }
};

}

#endif // GLU_AST_STMT_IFSTMT_HPP
