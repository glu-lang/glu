#ifndef GLU_SEMA_VALID_COPY_OVERLOAD_CHECKER_HPP
#define GLU_SEMA_VALID_COPY_OVERLOAD_CHECKER_HPP

#include "AST/ASTWalker.hpp"
#include "AST/TypePrinter.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class ValidCopyOverloadChecker
    : public ast::ASTWalker<ValidCopyOverloadChecker, void> {
    glu::DiagnosticManager &_diagManager;

public:
    ValidCopyOverloadChecker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void postVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        glu::ast::TypePrinter typePrinter;

        if (node->getName() != "copy") {
            return;
        }
        if (node->getParamCount() != 1) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'copy' overload: expected 1 parameter, got "
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
                "Invalid 'copy' overload: parameter must be a pointer to a "
                "struct type"
            );
            return;
        }

        auto *paramType
            = llvm::dyn_cast<types::StructTy>(paramPtrType->getPointee());
        if (!paramType) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'copy' overload: parameter must be a pointer to a "
                "struct type"
            );
            return;
        }
        if (node->getType()->getReturnType() != paramType) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'copy' overload: unexpected return type, expected '"
                    + typePrinter.visit(paramType) + "'"
            );
        }
        if (paramType->getDecl()->getModule() != node->getModule()) {
            _diagManager.error(
                node->getLocation(),
                "Invalid 'copy' overload: cannot overload 'copy' for types "
                "from other modules"
            );
            return;
        }

        if (node->getVisibility() != paramType->getDecl()->getVisibility()) {
            _diagManager.warning(
                node->getLocation(),
                "Invalid 'copy' overload: 'copy' function should have the same "
                "visibility as its associated type"
            );
        }

        paramType->getDecl()->setCopyFunction(node);
    }
};
}

#endif // GLU_SEMA_VALID_COPY_OVERLOAD_CHECKER_HPP
