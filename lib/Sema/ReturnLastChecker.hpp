#ifndef GLU_SEMA_RETURNLASTCHECKER_HPP
#define GLU_SEMA_RETURNLASTCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

inline bool checkFunctionEndsWithReturn(
    glu::ast::FunctionDecl *func, glu::DiagnosticManager &diagManager
)
{
    auto *fnType = func->getType();

    if (llvm::isa<glu::types::VoidTy>(fnType->getReturnType()))
        return true;

    auto *body = func->getBody();

    if (!body) {
        return true;
    }

    if (auto *compound = llvm::dyn_cast<glu::ast::CompoundStmt>(body)) {
        llvm::ArrayRef<glu::ast::StmtBase *> stmts = compound->getStmts();
        if (stmts.empty()) {
            diagManager.warning(
                body->getLocation(),
                llvm::Twine("Function '") + func->getName().str()
                    + "' does not end with a return statement"
            );
            return false;
        }

        auto *last = stmts.back();
        if (!llvm::isa<glu::ast::ReturnStmt>(last)) {
            diagManager.warning(
                body->getLocation(),
                llvm::Twine("Function '") + func->getName().str()
                    + "' does not end with a return statement"
            );
            return false;
        }

        return true;
    }

    // if (!llvm::isa<glu::ast::ReturnStmt>(body)) {
    //     diagManager.warning(
    //         body->getLocation(),
    //         llvm::Twine("Function '") + func->getName().str()
    //             + "' does not end with a return statement333"
    //     );
    //     return false;
    // }

    return true;
}

} // namespace glu::sema

#endif // GLU_SEMA_RETURNLASTCHECKER_HPP
