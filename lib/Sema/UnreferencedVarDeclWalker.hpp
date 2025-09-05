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

    ~UnreferencedVarDeclWalker() { emitWarnings(); }

    /// @brief Track variable declarations
    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        _declaredVars.insert(varLet);
    }

    /// @brief Track variable usage
    void postVisitRefExpr(glu::ast::RefExpr *node)
    {
        if (auto *assign
            = llvm::dyn_cast_or_null<glu::ast::AssignStmt>(node->getParent());
            assign && assign->getExprLeft() == node) {
            return;
        }

        if (auto *varDecl
            = llvm::dyn_cast_or_null<glu::ast::VarLetDecl *>(node->getVariable()
            )) {
            _usedVars.insert(varDecl);
        }
    }

private:
    void emitWarnings()
    {
        for (auto const *var : _declaredVars) {
            if (!_usedVars.contains(var)) {
                if (llvm::isa<glu::ast::ParamDecl>(var)) {
                    continue;
                }

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
