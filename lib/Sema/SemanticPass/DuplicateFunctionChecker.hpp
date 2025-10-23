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
    llvm::SmallVector<llvm::StringRef, 16> _linkageNameFunctionNames;

public:
    explicit DuplicateFunctionChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        if (node->getName() == "main")
            return;

        if (node->hasAttribute(ast::AttributeKind::NoManglingKind))
            checkNoManglingDuplicate(node);
        else if (node->hasAttribute(ast::AttributeKind::LinkageNameKind))
            checkLinkageNameDuplicate(node);
        else
            checkDuplicateFunction(node);
    }

private:
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

        // Also check if this no_mangling function conflicts with any linkage
        // names
        if (llvm::find(_linkageNameFunctionNames, functionName)
            != _linkageNameFunctionNames.end()) {
            _diagManager.error(
                node->getLocation(),
                "function with no_mangling conflicts with a function using "
                "linkage_name '"
                    + functionName.str() + "'"
            );
        }
    }

    void checkLinkageNameDuplicate(ast::FunctionDecl *node)
    {
        auto *linkageAttr
            = node->getAttribute(ast::AttributeKind::LinkageNameKind);
        if (!linkageAttr || !linkageAttr->getParameter())
            return;

        auto *literal
            = llvm::dyn_cast<ast::LiteralExpr>(linkageAttr->getParameter());
        if (!literal
            || !std::holds_alternative<llvm::StringRef>(literal->getValue()))
            return;

        llvm::StringRef linkageName
            = std::get<llvm::StringRef>(literal->getValue());

        if (llvm::find(_linkageNameFunctionNames, linkageName)
            != _linkageNameFunctionNames.end()) {
            _diagManager.error(
                node->getLocation(),
                "duplicate function with linkage_name '" + linkageName.str()
                    + "'"
            );
        } else {
            _linkageNameFunctionNames.push_back(linkageName);
        }

        // Also check if this linkage name conflicts with any no_mangling
        // functions
        if (llvm::find(_noManglingFunctionNames, linkageName)
            != _noManglingFunctionNames.end()) {
            _diagManager.error(
                node->getLocation(),
                "function with linkage_name '" + linkageName.str()
                    + "' conflicts with a no_mangling function"
            );
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
