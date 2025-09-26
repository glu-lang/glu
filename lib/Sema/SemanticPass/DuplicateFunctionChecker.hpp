#ifndef GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONCHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks a module. Checks the attributes on each declaration,
/// and emits diagnostics for invalid attributes.
class DuplicateFunctionChecker
    : public ast::ASTWalker<DuplicateFunctionChecker, void> {
    DiagnosticManager &_diagManager;
    llvm::SmallVector<llvm::StringRef, 16> _noManglingFunctionNames;

public:
    explicit DuplicateFunctionChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        if (node->getName() == "main")
            return;

        if (hasNoManglingAttribute(node))
            checkNoManglingDuplicate(node);
        else
            checkDuplicateFunction(node);
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

    void checkNoManglingDuplicate(ast::FunctionDecl *node)
    {
        llvm::StringRef functionName = node->getName();

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

    void checkDuplicateFunction(ast::FunctionDecl *node)
    {
        auto *module = node->getModule();
        if (!module)
            return;

        auto functions = module->getDeclsOfType<ast::FunctionDecl>();

        for (auto *func : functions) {
            if (func == node)
                continue;
            if (func->getName() == node->getName()
                && func->getType() == node->getType()) {
                _diagManager.error(
                    node->getLocation(),
                    "duplicate function declaration: " + node->getName().str()
                );
                return;
            }
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_DUPLICATEFUNCTIONCHECKER_HPP
