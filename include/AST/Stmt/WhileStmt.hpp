#ifndef GLU_AST_STMT_WHILESTMT_HPP
#define GLU_AST_STMT_WHILESTMT_HPP

#include "ASTNode.hpp"

#include "Stmt/CompoundStmt.hpp"

#include <cassert>

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
        assert(_condition && "Condition cannot be null.");
        assert(_body && "Body cannot be null.");
        condition->setParent(this);
        body->setParent(this);
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

    /// @brief Set the condition of the while statement.
    /// @param condition The new condition for the while statement.
    void setCondition(ExprBase *condition)
    {
        if (_condition != nullptr) {
            _condition->setParent(nullptr);
        }
        _condition = condition;
        if (_condition != nullptr) {
            _condition->setParent(this);
        }
    }

    /// @brief Set the body of the while statement.
    /// @param body The new body for the while statement.
    void setBody(CompoundStmt *body)
    {
        if (_body != nullptr) {
            _body->setParent(nullptr);
        }
        _body = body;
        if (_body != nullptr) {
            _body->setParent(this);
        }
    }
};

}

#endif // GLU_AST_STMT_WHILESTMT_HPP
