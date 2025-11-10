#ifndef GLU_SEMA_SEMANTICPASS_VALIDTYPECHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_VALIDTYPECHECKER_HPP

#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class ValidTypeChecker : public ast::ASTWalker<ValidTypeChecker, void> {
    DiagnosticManager &_diagManager;

public:
    ValidTypeChecker(DiagnosticManager &diagManager) : _diagManager(diagManager)
    {
    }

    void preVisitVarLetDecl(ast::VarLetDecl *node)
    {
        if (llvm::isa_and_nonnull<types::VoidTy>(node->getType())) {
            _diagManager.error(
                node->getLocation(),
                "variable or constant cannot be of type Void"
            );
        }
    }

    void preVisitEnumDecl(ast::EnumDecl *node)
    {
        auto *repr = node->getRepresentableType();
        if (!repr)
            return;
        auto *canonical
            = repr->getCanonicalType(*node->getModule()->getContext());
        if (!llvm::isa<types::IntTy>(canonical)
            && !llvm::isa<types::CharTy>(canonical)) {
            _diagManager.error(
                node->getLocation(),
                "enum representation type must be an integer or character type"
            );
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDTYPECHECKER_HPP
