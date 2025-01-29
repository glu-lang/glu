#ifndef GLU_AST_STMT_BREAKSTMT_HPP
#define GLU_AST_STMT_BREAKSTMT_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @class BreakStmt
/// @brief Represents a break statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a break
/// statement.
class BreakStmt : public StmtBase {
public:
    BreakStmt(SourceLocation location, ASTNode *parent)
        : StmtBase(NodeKind::BreakStmtKind, location, parent)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::BreakStmtKind;
    }
};

}

#endif // GLU_AST_STMT_BREAKSTMT_HPP
