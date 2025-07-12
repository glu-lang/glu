#ifndef GLU_AST_STMT_RETURNSTMT_HPP
#define GLU_AST_STMT_RETURNSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

namespace glu::ast {

/// @class ReturnStmt
/// @brief Represents a return statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a return
/// statement.
class ReturnStmt : public StmtBase {

    GLU_AST_GEN_CHILD(ReturnStmt, ExprBase *, _returnExpr, ReturnExpr)

public:
    /// @brief Constructor for the ReturnStmt class.
    /// @param location The source location of the compound statement.
    /// @param returnExpr The expression to return.
    ReturnStmt(SourceLocation location, ExprBase *returnExpr = nullptr)
        : StmtBase(NodeKind::ReturnStmtKind, location)
    {
        initReturnExpr(returnExpr, /* nullable = */ true);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ReturnStmtKind;
    }
};

}

#endif // GLU_AST_STMT_RETURNSTMT_HPP
