#ifndef GLU_AST_STMT_IFSTMT_HPP
#define GLU_AST_STMT_IFSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class IfStmt
/// @brief Represents a if statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a if
/// statement.
class IfStmt : public StmtBase {

    GLU_AST_GEN_CHILD(IfStmt, ExprBase *, _condition, Condition)
    GLU_AST_GEN_CHILD(IfStmt, CompoundStmt *, _body, Body)
    GLU_AST_GEN_CHILD(IfStmt, StmtBase *, _else, Else)

public:
    /// @brief Constructor for the IfStmt class.
    /// @param location The source location of the if keyword.
    /// @param condition The condition of the if statement.
    /// @param body The body of the if statement.
    /// @param elseBranch The else branch of the if statement, or nullptr if
    /// there is none.
    IfStmt(
        SourceLocation location, ExprBase *condition, CompoundStmt *body,
        StmtBase *elseBranch = nullptr
    )
        : StmtBase(NodeKind::IfStmtKind, location)
    {
        initCondition(condition);
        initBody(body);
        initElse(elseBranch, /* nullable = */ true);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::IfStmtKind;
    }
};

}

#endif // GLU_AST_STMT_IFSTMT_HPP
