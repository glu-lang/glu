#ifndef GLU_AST_STMT_EXPRESSIONSTMT_HPP
#define GLU_AST_STMT_EXPRESSIONSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

namespace glu::ast {

/// @class ExpressionStmt
/// @brief Represents an expression statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of an
/// expression statement.
class ExpressionStmt : public StmtBase {

    GLU_AST_GEN_CHILD(ExpressionStmt, ExprBase *, _expr, Expr)

public:
    /// @brief Constructor for the ExpressionStmt class.
    /// @param location The source location of the expression statement.
    /// @param expr The expression associated with this statement.
    ExpressionStmt(SourceLocation location, ExprBase *expr)
        : StmtBase(NodeKind::ExpressionStmtKind, location)
    {
        initExpr(expr);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ExpressionStmtKind;
    }
};

}

#endif // GLU_AST_STMT_EXPRESSIONSTMT_HPP
