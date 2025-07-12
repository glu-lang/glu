#ifndef GLU_AST_STMT_DECLSTMT_HPP
#define GLU_AST_STMT_DECLSTMT_HPP

#include "ASTNode.hpp"

namespace glu::ast {

/// @class DeclStmt
/// @brief Represents a decl statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a decl
/// statement.
class DeclStmt : public StmtBase {
    /// @brief The declaration associated with this statement.
    DeclBase *_decl;

public:
    DeclStmt(SourceLocation location, DeclBase *decl)
        : StmtBase(NodeKind::DeclStmtKind, location), _decl(decl)
    {
        assert(_decl && "Declaration cannot be null.");
        _decl->setParent(this);
    }

    /// @brief Returns the declaration associated with this statement.
    DeclBase *getDecl() const { return _decl; }

    /// @brief Sets the declaration for this statement.
    void setDecl(DeclBase *decl)
    {
        _decl = decl;
        if (_decl)
            _decl->setParent(this);
    }

    /// @brief Check if the given node is a decl statement.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::DeclStmtKind;
    }
};

}

#endif // GLU_AST_STMT_DECLSTMT_HPP
