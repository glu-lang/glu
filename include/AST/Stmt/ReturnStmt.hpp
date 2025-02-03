#ifndef GLU_AST_STMT_RETURNSTMT_HPP
#define GLU_AST_STMT_RETURNSTMT_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @class ReturnStmt
/// @brief Represents a return statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a return
/// statement.
class ReturnStmt : public StmtBase {
    /// @brief The expression to return.
    ExprBase *_returnExpr;

public:
    /// @brief Constructor for the ReturnStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param returnExpr The expression to return.
    ReturnStmt(
        SourceLocation location, ASTNode *parent, ExprBase *returnExpr = nullptr
    )
        : StmtBase(NodeKind::ReturnStmtKind, location, parent)
        , _returnExpr(returnExpr)
    {
    }

    /// @brief Get the expression to return.
    /// @return The expression to return.
    ExprBase *getReturnExpr() { return _returnExpr; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ReturnStmtKind;
    }
};

}

#endif // GLU_AST_STMT_RETURNSTMT_HPP
