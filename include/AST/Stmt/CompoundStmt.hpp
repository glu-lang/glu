#ifndef GLU_AST_STMT_COMPOUNDSTMT_HPP
#define GLU_AST_STMT_COMPOUNDSTMT_HPP

#include "ASTNode.hpp"

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
    using TrailingArgs = llvm::TrailingObjects<CompoundStmt, StmtBase *>;
    friend TrailingArgs;

    unsigned _stmtCount;

    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t
        numTrailingObjects(typename TrailingArgs::OverloadToken<StmtBase *>) const
    {
        return _stmtCount;
    }

    /// @brief Constructor for the CompoundStmt class.
    /// @param location The source location of the compound statement.
    /// @param stmts A vector of StmtBase pointers representing the statements
    /// in the compound statement.
    CompoundStmt(SourceLocation location, llvm::ArrayRef<StmtBase *> stmts)
        : StmtBase(NodeKind::CompoundStmtKind, location)
        , _stmtCount(stmts.size())
    {
        for (auto *stmt : stmts)
            stmt->setParent(this);
        std::uninitialized_copy(
            stmts.begin(), stmts.end(),
            this->template getTrailingObjects<StmtBase *>()
        );
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

    /// @brief Get the list of statements in the compound statement.
    /// @return A reference to the list of statements.
    llvm::ArrayRef<StmtBase *> getStmts()
    {
        return { getTrailingObjects<StmtBase *>(), _stmtCount };
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
