#ifndef GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONNOMANGLINGCHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONNOMANGLINGCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks a module. Checks the attributes on each declaration,
/// and emits diagnostics for invalid attributes.
class DuplicateFunctionNoManglingChecker
    : public ast::ASTWalker<DuplicateFunctionNoManglingChecker, void> {
    DiagnosticManager &_diagManager;
    llvm::SmallVector<llvm::StringRef, 16> _noManglingFunctionNames;

public:
    explicit DuplicateFunctionNoManglingChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        if (!hasNoManglingAttribute(node))
            return;
        llvm::StringRef functionName = node->getName();

        if (functionName == "main")
            return;

        // Check for duplicates
        if (llvm::find(_noManglingFunctionNames, functionName)
            != _noManglingFunctionNames.end()) {
            _diagManager.error(
                node->getLocation(),
                "duplicate function with no_mangling attribute: "
                    + functionName.str()
            );
        } else {
            _noManglingFunctionNames.push_back(functionName);
        }
    }

private:
    bool hasNoManglingAttribute(ast::FunctionDecl *node)
    {
        auto *attrs = node->getAttributes();

        if (!attrs)
            return false;
        return llvm::any_of(attrs->getAttributes(), [](auto const *attr) {
            return attr->getAttributeKind()
                == ast::AttributeKind::NoManglingKind;
        });
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONNOMANGLINGCHECKER_HPP
