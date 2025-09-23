#ifndef GLU_SEMA_SEMANTICPASS_UNREFERENCEDVARDECLWALKER_HPP
#define GLU_SEMA_SEMANTICPASS_UNREFERENCEDVARDECLWALKER_HPP

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

    void postVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        if (!node->getBody()) {
            _declaredVars.clear();
        }
    }

    /// @brief Track variable declarations
    void postVisitVarLetDecl(glu::ast::VarLetDecl *varLet)
    {
        _declaredVars.insert(varLet);
    }

    /// @brief Track variable usage
    void postVisitRefExpr(ast::RefExpr *node)
    {
        if (auto *assign
            = llvm::dyn_cast_if_present<ast::AssignStmt>(node->getParent());
            assign && assign->getExprLeft() == node) {
            return;
        }

        auto const &referenced = node->getVariable();
        if (!referenced)
            return;
        if (auto *varDecl = llvm::dyn_cast<ast::VarLetDecl *>(referenced)) {
            _usedVars.insert(varDecl);
        }
    }

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

#endif // GLU_SEMA_SEMANTICPASS_UNREFERENCEDVARDECLWALKER_HPP
