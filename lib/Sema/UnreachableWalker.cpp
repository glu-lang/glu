#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks compound blocks and emits warnings for statements that are
/// unreachable because they follow a return, break or continue.
class UnreachableWalker : public glu::ast::ASTWalker<UnreachableWalker, void> {
    glu::DiagnosticManager &_diagManager;
    /// Stack indicating whether the current compound scope has become
    /// unreachable.
    llvm::SmallVector<bool, 8> _scopeUnreachable;

public:
    explicit UnreachableWalker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    /// @brief Enter a new compound scope (initially reachable)
    void preVisitCompoundStmt(glu::ast::CompoundStmt *)
    {
        _scopeUnreachable.push_back(false);
    }

    /// @brief Leave compound scope
    void postVisitCompoundStmt(glu::ast::CompoundStmt *)
    {
        if (!_scopeUnreachable.empty())
            _scopeUnreachable.pop_back();
    }

    /// @brief If a return is seen, the remainder of the current compound is
    /// unreachable
    void postVisitReturnStmt(glu::ast::ReturnStmt *)
    {
        if (!_scopeUnreachable.empty())
            _scopeUnreachable.back() = true;
    }

    /// @brief break/continue also render subsequent statements in the same
    /// compound unreachable
    void postVisitBreakStmt(glu::ast::BreakStmt *)
    {
        if (!_scopeUnreachable.empty())
            _scopeUnreachable.back() = true;
    }

    void postVisitContinueStmt(glu::ast::ContinueStmt *)
    {
        if (!_scopeUnreachable.empty())
            _scopeUnreachable.back() = true;
    }

    /// @brief For every statement about to be visited, if the current compound
    /// scope is already marked unreachable, emit a warning.
    void preVisitStmtBase(glu::ast::StmtBase *stmt)
    {
        if (_scopeUnreachable.empty())
            return;

        if (_scopeUnreachable.back()) {
            _diagManager.warning(
                stmt->getLocation(),
                llvm::Twine(
                    "Unreachable code: this statement is never executed"
                )
            );
        }
    }
};

} // namespace glu::sema
