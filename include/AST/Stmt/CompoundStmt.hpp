#ifndef GLU_AST_STMT_COMPOUNDSTMT_HPP
#define GLU_AST_STMT_COMPOUNDSTMT_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::ast {

/// @class CompoundStmt
/// @brief Represents a compound statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a compound
/// statement, including its list of statements.
class CompoundStmt : public StmtBase {
    llvm::SmallVector<StmtBase *> _stmts;

public:
    /// @brief Constructor for the CompoundStmt class.
    /// @param location The source location of the compound statement.
    /// @param parent The parent AST node.
    /// @param stmts A vector of StmtBase pointers representing the statements
    /// in the compound statement.
    CompoundStmt(
        SourceLocation location, ASTNode *parent,
        llvm::SmallVector<StmtBase *> stmts
    )
        : StmtBase(NodeKind::CompoundStmtKind, location, parent)
        , _stmts(std::move(stmts))
    {
    }

    /// @brief Get the list of statements in the compound statement.
    /// @return A reference to the list of statements.
    llvm::ArrayRef<StmtBase *> getStmts() const { return _stmts; }

    /// @brief Get the list of statements in the compound statement.
    /// @return A reference to the list of statements.
    llvm::ArrayRef<StmtBase *> getStmts() { return _stmts; }
};

} // namespace glu::ast

#endif // GLU_AST_STMT_COMPOUNDSTMT_HPP
