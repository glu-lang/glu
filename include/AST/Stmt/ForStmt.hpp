#ifndef GLU_AST_STMT_FORSTMT_HPP
#define GLU_AST_STMT_FORSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include "ASTNode.hpp"
#include "Decl/ForBindingDecl.hpp"
#include "Expr/RefExpr.hpp"
#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class ForStmt
/// @brief Represents a for statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a for
/// statement.
class ForStmt : public StmtBase {

    GLU_AST_GEN_CHILD(ForStmt, ForBindingDecl *, _binding, Binding)
    GLU_AST_GEN_CHILD(ForStmt, ExprBase *, _range, Range)
    GLU_AST_GEN_CHILD(ForStmt, CompoundStmt *, _body, Body)
    GLU_AST_GEN_CHILD(ForStmt, RefExpr *, _beginFunc, BeginFunc)
    GLU_AST_GEN_CHILD(ForStmt, RefExpr *, _endFunc, EndFunc)
    GLU_AST_GEN_CHILD(ForStmt, RefExpr *, _nextFunc, NextFunc)
    GLU_AST_GEN_CHILD(ForStmt, RefExpr *, _derefFunc, DerefFunc)
    GLU_AST_GEN_CHILD(ForStmt, RefExpr *, _equalityFunc, EqualityFunc)

public:
    /// @brief Constructor for the ForStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param binding The binding of the for statement.
    /// @param body The body of the for statement.
    /// @param range The range of the for statement.
    ForStmt(
        SourceLocation location, ForBindingDecl *binding, ExprBase *range,
        CompoundStmt *body
    )
        : StmtBase(NodeKind::ForStmtKind, location)
    {
        initBinding(binding);
        initRange(range);
        initBody(body);
        initBeginFunc(nullptr, /*nullable=*/true);
        initEndFunc(nullptr, /*nullable=*/true);
        initNextFunc(nullptr, /*nullable=*/true);
        initDerefFunc(nullptr, /*nullable=*/true);
        initEqualityFunc(nullptr, /*nullable=*/true);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ForStmtKind;
    }
};
}

#endif // GLU_AST_STMT_FORSTMT_HPP
