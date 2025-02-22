#ifndef GLU_AST_STMT_WHILESTMT_HPP
#define GLU_AST_STMT_WHILESTMT_HPP

#include "ASTNode.hpp"

#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class WhileStmt
/// @brief Represents a while statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a while
/// statement.
class WhileStmt : public StmtBase {
    /// @brief The condition of the while statement.
    ExprBase *_condition;
    /// @brief The body of the while statement.
    CompoundStmt *_body;

public:
    /// @brief Constructor for the WhileStmt class.
    /// @param location The source location of the while keyword.
    /// @param condition The condition of the while statement.
    /// @param body The body of the while statement.
    WhileStmt(SourceLocation location, ExprBase *condition, CompoundStmt *body)
        : StmtBase(NodeKind::WhileStmtKind, location)
        , _condition(condition)
        , _body(body)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::WhileStmtKind;
    }

    /// @brief Get the condition of the while statement.
    /// @return The condition of the while statement.
    ExprBase *getCondition() { return _condition; }

    /// @brief Get the body of the while statement.
    /// @return The body of the while statement.
    CompoundStmt *getBody() { return _body; }
};

}

#endif // GLU_AST_STMT_WHILESTMT_HPP
