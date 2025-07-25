#ifndef GLU_AST_STMT_COMPOUNDSTMT_HPP
#define GLU_AST_STMT_COMPOUNDSTMT_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @class CompoundStmt
/// @brief Represents a compound statement in the AST.
///
/// This class inherits from StmtBase and encapsulates the details of a compound
/// statement, including its list of statements.
class CompoundStmt final
    : public StmtBase,
      private llvm::TrailingObjects<CompoundStmt, StmtBase *> {

    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(
        CompoundStmt, _stmtCount, StmtBase *, Stmts
    )

private:
    /// @brief Constructor for the CompoundStmt class.
    /// @param location The source location of the compound statement.
    /// @param stmts A vector of StmtBase pointers representing the statements
    /// in the compound statement.
    CompoundStmt(SourceLocation location, llvm::ArrayRef<StmtBase *> stmts)
        : StmtBase(NodeKind::CompoundStmtKind, location)
    {
        initStmts(stmts);
    }

public:
    /// @brief Static method to create a new CompoundStmt.
    /// @param alloc The allocator used to create the CompoundStmt.
    /// @param location The source location of the compound statement.
    /// @param stmts A vector of StmtBase pointers representing the statements
    /// in the compound statement.
    /// @return A pointer to the newly created CompoundStmt.
    static CompoundStmt *create(
        llvm::BumpPtrAllocator &alloc, SourceLocation location,
        llvm::ArrayRef<StmtBase *> stmts
    )
    {
        auto totalSize = totalSizeToAlloc<StmtBase *>(stmts.size());
        void *mem = alloc.Allocate(totalSize, alignof(CompoundStmt));

        return new (mem) CompoundStmt(location, stmts);
    }

    /// @brief Check if the given node is a compound statement.
    /// @param node The node to check.
    /// @return True if the node is a compound statement, false otherwise.
    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::CompoundStmtKind;
    }
};

} // namespace glu::ast

#endif // GLU_AST_STMT_COMPOUNDSTMT_HPP
