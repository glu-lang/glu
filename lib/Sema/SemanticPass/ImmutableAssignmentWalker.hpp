#ifndef GLU_SEMA_SEMANTICPASS_IMMUTABLEASSIGNMENTWALKER_HPP
#define GLU_SEMA_SEMANTICPASS_IMMUTABLEASSIGNMENTWALKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks the AST and emits errors for assignments to immutable
/// variables.
class ImmutableAssignmentWalker
    : public glu::ast::ASTWalker<ImmutableAssignmentWalker, void> {
    glu::DiagnosticManager &_diagManager;

public:
    explicit ImmutableAssignmentWalker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    /// @brief Check assignments for immutability violations.
    void postVisitAssignStmt(glu::ast::AssignStmt *assignStmt)
    {
        auto *lhs = assignStmt->getExprLeft();

        auto *refExpr = llvm::dyn_cast<glu::ast::RefExpr>(lhs);
        if (!refExpr)
            return;

        auto *varDecl
            = llvm::dyn_cast<glu::ast::VarLetDecl *>(refExpr->getVariable());
        if (!varDecl)
            return;

        if (llvm::isa<glu::ast::LetDecl>(varDecl)
            || llvm::isa<glu::ast::ParamDecl>(varDecl)
            || llvm::isa<glu::ast::ForBindingDecl>(varDecl)) {
            _diagManager.error(
                assignStmt->getLocation(),
                llvm::Twine("Cannot assign to immutable variable '")
                    + varDecl->getName().str() + "'"
            );
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_IMMUTABLEASSIGNMENTWALKER_HPP
