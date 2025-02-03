#ifndef GLU_AST_STMT_WHILESTMT_HPP
#define GLU_AST_STMT_WHILESTMT_HPP

#include "ASTNode.hpp"

#include "ASTNode.hpp"
#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class ForStmt
/// @brief Represents a for statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a for
/// statement.
class ForStmt : public StmtBase {
    /// @brief The condition of the for statement.
    ExprBase *_condition;
    /// @brief The body of the for statement.
    ExprBase *_init;
    CompoundStmt *_body;
    ExprBase *_increment;

public:
    /// @brief Constructor for the ForStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param condition The condition of the for statement.
    /// @param init The init of the for statement.
    /// @param body The body of the for statement.
    /// @param increment The increment of the for statement.
    ForStmt(
        SourceLocation location, ASTNode *parent, ExprBase *condition,
        ExprBase *init, CompoundStmt *body, ExprBase *increment
    )
        : StmtBase(NodeKind::ForStmtKind, location, parent)
        , _condition(condition)
        , _init(init)
        , _body(body)
        , _increment(increment)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ForStmtKind;
    }

    /// @brief Get the condition of the for statement.
    /// @return The condition of the for statement.
    ExprBase *getCondition() { return _condition; }

    /// @brief Get the init of the for statement.
    /// @return The init of the for statement.
    ExprBase *getInit() { return _init; }

    /// @brief Get the increment of the for statement.
    /// @return The increment of the for statement.
    ExprBase *getIncrement() { return _increment; }

    /// @brief Get the body of the for statement.
    /// @return The body of the for statement.
    CompoundStmt *getBody() { return _body; }
};
}

#endif // GLU_AST_STMT_WHILESTMT_HPP
