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
        size_t neededFieldCount = structType->getNeededFieldCount();

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
