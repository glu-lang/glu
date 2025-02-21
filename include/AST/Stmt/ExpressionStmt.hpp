#ifndef GLU_AST_STMT_EXPRESSIONSTMT_HPP
#define GLU_AST_STMT_EXPRESSIONSTMT_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @class ExpressionStmt
/// @brief Represents an expression statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of an
/// expression statement.
class ExpressionStmt : public StmtBase {
    /// @brief Pointer to the expression associated with this statement.
    ExprBase *_expr;

public:
    /// @brief Constructor for the ExpressionStmt class.
    /// @param location The source location of the expression statement.
    /// @param expr The expression associated with this statement.
    ExpressionStmt(SourceLocation location, ExprBase *expr)
        : StmtBase(NodeKind::ExpressionStmtKind, location), _expr(expr)
    {
        assert(_expr && "Expression cannot be null.");
        _expr->setParent(this);
    }

    /// @brief Get the expression associated with this statement.
    /// @return The expression associated with this statement.
    ExprBase *getExpr() { return _expr; }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ExpressionStmtKind;
    }
};

}

#endif // GLU_AST_STMT_EXPRESSIONSTMT_HPP
