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
    llvm::SmallVector<llvm::StringRef, 16> _usedLinkageNames;

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
            checkLinkageNameDuplicate(node, node->getName(), "no_mangling");
        else if (node->hasAttribute(ast::AttributeKind::LinkageNameKind))
            checkLinkageNameFromAttribute(node);
        else
            checkDuplicateFunction(node);
    }

private:
    /// @brief Check for duplicate linkage names (used by both @no_mangling and
    /// @linkage_name)
    void checkLinkageNameDuplicate(
        ast::FunctionDecl *node, llvm::StringRef linkageName,
        llvm::StringRef attributeType
    )
    {
        if (llvm::find(_usedLinkageNames, linkageName)
            != _usedLinkageNames.end()) {
            _diagManager.error(
                node->getLocation(),
                "duplicate function with " + attributeType.str()
                    + " attribute '" + linkageName.str() + "'"
            );
        } else {
            _usedLinkageNames.push_back(linkageName);
        }
    }

    /// @brief Handle @linkage_name attribute specifically
    void checkLinkageNameFromAttribute(ast::FunctionDecl *node)
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
        checkLinkageNameDuplicate(node, linkageName, "linkage_name");
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
