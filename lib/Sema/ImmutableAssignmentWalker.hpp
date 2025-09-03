#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

void ImmutableAssignmentWalker::postVisitAssignStmt(
    glu::ast::AssignStmt *assignStmt
)
{
    // Get the left-hand side of the assignment
    auto *lhs = assignStmt->getExprLeft();

    // Ensure the left-hand side is a RefExpr
    auto *refExpr = llvm::dyn_cast<glu::ast::RefExpr>(lhs);
    if (!refExpr)
        return;

    // Get the variable declaration associated with the RefExpr
    auto varDecl
        = refExpr->getVariable().template get<glu::ast::VarLetDecl *>();
    if (!varDecl)
        return;

    // Check if the variable is immutable (LetDecl)
    if (llvm::isa<glu::ast::LetDecl>(varDecl)) {
        _diagManager.error(
            assignStmt->getLocation(),
            llvm::Twine("Cannot assign to immutable variable '")
                + varDecl->getName().str() + "'"
        );
    }
}

} // namespace glu::sema
