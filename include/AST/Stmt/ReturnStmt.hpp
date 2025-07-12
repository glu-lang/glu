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
    /// @param returnExpr The expression to return.
    ReturnStmt(SourceLocation location, ExprBase *returnExpr = nullptr)
        : StmtBase(NodeKind::ReturnStmtKind, location), _returnExpr(returnExpr)
    {
        if (returnExpr)
            returnExpr->setParent(this);
    }

    /// @brief Get the expression to return.
    /// @return The expression to return.
    ExprBase *getReturnExpr() { return _returnExpr; }

    /// @brief Set the expression to return.
    /// @param returnExpr The expression to return.
    void setReturnExpr(ExprBase *returnExpr)
    {
        if (_returnExpr != nullptr) {
            _returnExpr->setParent(nullptr);
        }
        _returnExpr = returnExpr;
        if (_returnExpr)
            _returnExpr->setParent(this);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ReturnStmtKind;
    }
};

}

#endif // GLU_AST_STMT_RETURNSTMT_HPP
