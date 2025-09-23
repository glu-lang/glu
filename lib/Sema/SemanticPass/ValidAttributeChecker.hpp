#ifndef GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks compound blocks and emits warnings for statements that are
/// unreachable because they follow a return, break or continue.
class ValidAttributeChecker
    : public ast::ASTWalker<ValidAttributeChecker, void> {
    DiagnosticManager &_diagManager;

public:
    explicit ValidAttributeChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        for (auto *attr : node->getAttributes()->getAttributes()) {
            auto attachment = node->getBody()
                ? ast::AttributeAttachment::FunctionDefinitionAttachment
                : ast::AttributeAttachment::FunctionPrototypeAttachment;
            if (!attr->isValidOn(attachment)) {
                llvm::StringRef location = "function declarations";
                if (attr->isValidOnOneOf(
                        ast::AttributeAttachment::FunctionAttachment
                    )) {
                    location = node->getBody() ? "function definitions"
                                               : "function prototypes";
                }
                _diagManager.error(
                    attr->getLocation(),
                    llvm::Twine("Attribute '@")
                        + attr->getAttributeKindSpelling()
                        + "' is not valid on " + location
                );
            }
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDATTRIBUTECHECKER_HPP
