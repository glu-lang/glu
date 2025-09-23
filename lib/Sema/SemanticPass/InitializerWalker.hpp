#ifndef GLU_SEMA_SEMANTICPASS_INITIALIZERWALKER_HPP
#define GLU_SEMA_SEMANTICPASS_INITIALIZERWALKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

class InitializerWalker : public glu::ast::ASTWalker<InitializerWalker, void> {
    glu::DiagnosticManager &_diagManager;

public:
    InitializerWalker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitStructDecl(glu::ast::StructDecl *node)
    {
        bool foundFieldWithDefault = false;
        SourceLocation location = SourceLocation::invalid;

        for (auto *field : node->getFields()) {
            if (field->getValue() != nullptr) {
                foundFieldWithDefault = true;
                location = field->getLocation();
            } else if (foundFieldWithDefault) {
                _diagManager.error(
                    location,
                    "Fields with default values must come after all fields "
                    "without defaults"
                );
                return;
            }
        }
    }

    void postVisitStructInitializerExpr(glu::ast::StructInitializerExpr *node)
    {
        auto *structType
            = llvm::dyn_cast<glu::types::StructTy>(node->getType());
        if (!structType) {
            // Type constraint will handle this error
            return;
        }

        auto providedFields = node->getFields();
        auto structFields = structType->getFields();
        size_t neededFieldCount = structType->getRequiredFieldCount();

        // Check if too many fields are provided
        if (providedFields.size() > structFields.size()) {
            _diagManager.error(
                node->getLocation(),
                llvm::Twine("Too many initializers for struct '")
                    + structType->getName() + "' (expected at most "
                    + llvm::Twine(structFields.size()) + ", got "
                    + llvm::Twine(providedFields.size()) + ")"
            );
            return;
        }

        // Check if enough fields are provided (considering defaults)
        if (providedFields.size() < neededFieldCount) {
            _diagManager.error(
                node->getLocation(),
                llvm::Twine("Not enough initializers for struct '")
                    + structType->getName() + "' (expected at least "
                    + llvm::Twine(neededFieldCount) + ", got "
                    + llvm::Twine(providedFields.size()) + ")"
            );
            return;
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_INITIALIZERWALKER_HPP
