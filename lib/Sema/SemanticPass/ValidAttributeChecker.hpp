#ifndef GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks a module. Checks the attributes on each declaration,
/// and emits diagnostics for invalid attributes.
class ValidAttributeChecker
    : public ast::ASTWalker<ValidAttributeChecker, void> {
    DiagnosticManager &_diagManager;

public:
    explicit ValidAttributeChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void check(
        ast::DeclBase *decl, ast::AttributeAttachment attachment,
        llvm::Twine description
    )
    {
        if (!decl->getAttributes())
            return;
        for (auto *attr : decl->getAttributes()->getAttributes()) {
            if (!attr->isValidOn(attachment)) {
                _diagManager.error(
                    attr->getLocation(),
                    llvm::Twine("Attribute '@")
                        + attr->getAttributeKindSpelling()
                        + "' is not valid on " + description
                );
            }
        }
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        if (node->getBody()) {
            check(
                node, ast::AttributeAttachment::FunctionDefinitionAttachment,
                "function definitions"
            );
        } else {
            check(
                node, ast::AttributeAttachment::FunctionPrototypeAttachment,
                "function prototypes"
            );
        }
    }

    void preVisitImportDecl(ast::ImportDecl *node)
    {
        check(node, ast::AttributeAttachment::ImportAttachment, "imports");
    }

    void preVisitStructDecl(ast::StructDecl *node)
    {
        check(node, ast::AttributeAttachment::StructAttachment, "structs");
    }

    void preVisitEnumDecl(ast::EnumDecl *node)
    {
        check(node, ast::AttributeAttachment::EnumAttachment, "enums");
    }

    void preVisitTypeAliasDecl(ast::TypeAliasDecl *node)
    {
        check(
            node, ast::AttributeAttachment::TypeAliasAttachment, "type aliases"
        );
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP
