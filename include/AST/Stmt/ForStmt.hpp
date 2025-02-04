#ifndef GLU_AST_STMT_WHILESTMT_HPP
#define GLU_AST_STMT_WHILESTMT_HPP

#include "ASTNode.hpp"

#include "ASTNode.hpp"
#include "Stmt/CompoundStmt.hpp"

namespace glu::ast {

/// @class ForStmt
/// @brief Represents a for statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a for
/// statement.
class ForStmt : public StmtBase {
    /// @brief The init of the for statement.
    ExprBase *_init;
    /// @brief The range of the for statement.
    ExprBase *_range;
    /// @brief The body of the for statement.
    CompoundStmt *_body;

public:
    /// @brief Constructor for the ForStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param init The init of the for statement.
    /// @param body The body of the for statement.
    /// @param range The range of the for statement.
    ForStmt(
        SourceLocation location, ASTNode *parent, ExprBase *init,
        ExprBase *range, CompoundStmt *body
    )
        : StmtBase(NodeKind::ForStmtKind, location, parent)
        , _init(init)
        , _range(range)
        , _body(body)
    {
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::ForStmtKind;
    }

    /// @brief Get the init of the for statement.
    /// @return The init of the for statement.
    ExprBase *getInit() { return _init; }

    /// @brief Get the range of the for statement.
    /// @return The range of the for statement.
    ExprBase *getRange() { return _range; }

    /// @brief Get the body of the for statement.
    /// @return The body of the for statement.
    CompoundStmt *getBody() { return _body; }
};
}

#endif // GLU_AST_STMT_WHILESTMT_HPP
