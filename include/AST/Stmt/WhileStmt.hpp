#ifndef GLU_AST_STMT_WHILESTMT_HPP
#define GLU_AST_STMT_WHILESTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include "Stmt/CompoundStmt.hpp"

#include <cassert>

namespace glu::ast {

/// @class WhileStmt
/// @brief Represents a while statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a while
/// statement.
class WhileStmt : public StmtBase {

    GLU_AST_GEN_CHILD(WhileStmt, ExprBase *, _condition, Condition)
    GLU_AST_GEN_CHILD(WhileStmt, CompoundStmt *, _body, Body)

public:
    /// @brief Constructor for the WhileStmt class.
    /// @param location The source location of the while keyword.
    /// @param condition The condition of the while statement.
    /// @param body The body of the while statement.
    WhileStmt(SourceLocation location, ExprBase *condition, CompoundStmt *body)
        : StmtBase(NodeKind::WhileStmtKind, location)
    {
        initCondition(condition);
        initBody(body);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::WhileStmtKind;
    }
};

}

#endif // GLU_AST_STMT_WHILESTMT_HPP
