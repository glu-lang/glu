#ifndef GLU_AST_STMT_CONTINUESTMT_HPP
#define GLU_AST_STMT_CONTINUESTMT_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @class ContinueStmt
/// @brief Represents a continue statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a continue
/// statement.
class ContinueStmt : public StmtBase {
public:
    ContinueStmt(SourceLocation location, ASTNode *parent)
        : StmtBase(NodeKind::ContinueStmtKind, location, parent)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ContinueStmtKind;
    }
};

}

#endif // GLU_AST_STMT_CONTINUESTMT_HPP
