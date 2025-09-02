#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks a function/block and emits warnings for declared but unused
/// variables.
class UnreferencedVarDeclWalker
    : public glu::ast::ASTWalker<UnreferencedVarDeclWalker, void> {
    glu::DiagnosticManager &_diagManager;

    // Track declared and used variables in the current scope
    llvm::SmallPtrSet<glu::ast::VarLetDecl const *, 16> _declaredVars;
    llvm::SmallPtrSet<glu::ast::VarLetDecl const *, 16> _usedVars;

public:
    explicit UnreferencedVarDeclWalker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    /// @brief Track variable declarations
    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        if (llvm::isa<glu::ast::ParamDecl>(varLet))
            return;
        _declaredVars.insert(varLet);
    }

    /// @brief Track variable usage
    void postVisitRefExpr(glu::ast::RefExpr *node)
    {
        if (auto *varDecl
            = llvm::dyn_cast<glu::ast::VarLetDecl *>(node->getVariable())) {
            _usedVars.insert(varDecl);
        }
    }

    /// @brief On leaving a block, emit warnings for unused variables and clear
    /// sets
    void postVisitCompoundStmt(glu::ast::CompoundStmt *) { emitWarnings(); }

private:
    void emitWarnings()
    {
        for (auto const *var : _declaredVars) {
            if (!_usedVars.contains(var)) {
                _diagManager.warning(
                    var->getLocation(),
                    llvm::Twine("Variable '") + var->getName().str()
                        + "' declared but not used"
                );
            }
        }
        _declaredVars.clear();
        _usedVars.clear();
    }
};

} // namespace
