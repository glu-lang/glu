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

private:
    /// @brief Validates the @alignment attribute parameter
    void validateAlignmentAttribute(ast::Attribute *attr)
    {
        if (!attr->getParameter())
            return;

        auto *literal = llvm::dyn_cast<ast::LiteralExpr>(attr->getParameter());
        if (!literal
            || !std::holds_alternative<llvm::APInt>(literal->getValue()))
            return;

        uint64_t alignment
            = std::get<llvm::APInt>(literal->getValue()).getZExtValue();

        // Check if alignment is a power of 2 and non-zero
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            _diagManager.error(
                attr->getLocation(),
                llvm::Twine("Alignment must be a power of 2, got ")
                    + llvm::Twine(alignment)
            );
            return;
        }

        // Check if alignment is reasonable (LLVM max is 2^29)
        if (alignment > (1ULL << 29)) {
            _diagManager.error(
                attr->getLocation(),
                llvm::Twine("Alignment ") + llvm::Twine(alignment)
                    + " is too large (maximum is " + llvm::Twine(1ULL << 29)
                    + ")"
            );
        }
    }

    /// @brief Validates attribute-specific constraints
    void validateAttributeValue(ast::Attribute *attr)
    {
        // TODO: made a visitor
        switch (attr->getAttributeKind()) {
        case ast::AttributeKind::AlignmentKind:
            validateAlignmentAttribute(attr);
            break;
        default: break;
        }
    }

    /// @brief Checks parameter validity (presence and type)
    void checkParameterValidity(ast::Attribute *attr)
    {
        if (attr->expectsParameter() && !attr->getParameter()) {
            _diagManager.error(
                attr->getLocation(),
                llvm::Twine("Attribute '@") + attr->getAttributeKindSpelling()
                    + "' expects a parameter of type "
                    + attr->getExpectedParameterTypeName()
            );
        } else if (!attr->expectsParameter() && attr->getParameter()) {
            _diagManager.error(
                attr->getLocation(),
                llvm::Twine("Attribute '@") + attr->getAttributeKindSpelling()
                    + "' does not accept a parameter"
            );
        } else if (attr->getParameter()
                   && !attr->isValidParameterType(attr->getParameter())) {
            _diagManager.error(
                attr->getLocation(),
                llvm::Twine("Attribute '@") + attr->getAttributeKindSpelling()
                    + "' expects a parameter of type "
                    + attr->getExpectedParameterTypeName()
                    + ", but got an incompatible expression"
            );
        }
    }

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
            // Check if attribute is valid on this declaration type
            if (!attr->isValidOn(attachment)) {
                _diagManager.error(
                    attr->getLocation(),
                    llvm::Twine("Attribute '@")
                        + attr->getAttributeKindSpelling()
                        + "' is not valid on " + description
                );
                continue;
            }

            // Check parameter validity (presence and type)
            checkParameterValidity(attr);

            // Validate attribute-specific constraints
            validateAttributeValue(attr);
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

    void preVisitVarDecl(ast::VarDecl *node)
    {
        if (node->isGlobal()) {
            check(
                node, ast::AttributeAttachment::GlobalVarAttachment,
                "global variables"
            );
        } else {
            check(
                node, ast::AttributeAttachment::LocalVarAttachment,
                "local variables"
            );
        }
    }

    void preVisitLetDecl(ast::LetDecl *node)
    {
        if (node->isGlobal()) {
            check(
                node, ast::AttributeAttachment::GlobalLetAttachment,
                "global constants"
            );
        } else {
            check(
                node, ast::AttributeAttachment::LocalLetAttachment,
                "local constants"
            );
        }
    }

    void preVisitParamDecl(ast::ParamDecl *node)
    {
        check(node, ast::AttributeAttachment::ParamAttachment, "parameters");
    }

    void preVisitFieldDecl(ast::FieldDecl *node)
    {
        check(node, ast::AttributeAttachment::FieldAttachment, "fields");
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP
