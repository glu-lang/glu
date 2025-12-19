#ifndef GLU_SEMA_VALID_DROP_OVERLOAD_CHECKER_HPP
#define GLU_SEMA_VALID_DROP_OVERLOAD_CHECKER_HPP

#include "AST/ASTWalker.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class ValidDropOverloadChecker
    : public ast::ASTWalker<ValidDropOverloadChecker, void> {
    glu::DiagnosticManager &_diagManager;

public:
    ValidDropOverloadChecker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void postVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        if (node->getName() != "drop") {
            return;
        }
        if (!llvm::isa<types::VoidTy>(node->getType()->getReturnType())) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'drop' overload: unexpected return type, expected "
                "'Void'"
            );
        }
        if (node->getParamCount() != 1) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'drop' overload: expected 1 parameter, got "
                    + std::to_string(node->getParamCount())
            );
            return;
        }

        // The parameter must be a pointer to a struct type
        auto *paramPtrType
            = llvm::dyn_cast<types::PointerTy>(node->getParams()[0]->getType());
        if (!paramPtrType) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'drop' overload: parameter must be a pointer to a "
                "struct type"
            );
            return;
        }

        auto *paramType
            = llvm::dyn_cast<types::StructTy>(paramPtrType->getPointee());
        if (!paramType) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'drop' overload: parameter must be a pointer to a "
                "struct type"
            );
            return;
        }
        if (paramType->getDecl()->getModule() != node->getModule()) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'drop' overload: cannot overload 'drop' for types "
                "from other modules"
            );
            return;
        }

        if (node->getVisibility() != paramType->getDecl()->getVisibility()) {
            _diagManager.warning(
                node->getLocation(),
                "Invalid 'drop' overload: 'drop' function should have the same "
                "visibility as its associated type"
            );
        }

        paramType->getDecl()->setDropFunction(node);
    }
};
}

#endif // GLU_SEMA_VALID_DROP_OVERLOAD_CHECKER_HPP
