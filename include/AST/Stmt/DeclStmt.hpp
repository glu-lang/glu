#ifndef GLU_AST_STMT_DECLSTMT_HPP
#define GLU_AST_STMT_DECLSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

namespace glu::ast {

/// @class DeclStmt
/// @brief Represents a decl statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a decl
/// statement.
class DeclStmt : public StmtBase {

    GLU_AST_GEN_CHILD(DeclStmt, DeclBase *, _decl, Decl)

public:
    DeclStmt(SourceLocation location, DeclBase *decl)
        : StmtBase(NodeKind::DeclStmtKind, location)
    {
        initDecl(decl);
    }

    /// @brief Check if the given node is a decl statement.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::DeclStmtKind;
    }
};

}

#endif // GLU_AST_STMT_DECLSTMT_HPP
