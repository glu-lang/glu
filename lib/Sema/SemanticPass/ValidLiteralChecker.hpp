#ifndef GLU_SEMA_SEMANTICPASS_VALIDLITERALCHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_VALIDLITERALCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Walks a function and checks the validity of literal expressions.
class ValidLiteralChecker : public ast::ASTWalker<ValidLiteralChecker, void> {
    DiagnosticManager &_diagManager;

public:
    explicit ValidLiteralChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitLiteralExpr(ast::LiteralExpr *node)
    {
        // String literal for a character type must be a single character
        if (llvm::isa<types::CharTy>(node->getType())
            && std::holds_alternative<llvm::StringRef>(node->getValue())) {
            auto strValue = std::get<llvm::StringRef>(node->getValue());
            if (strValue.size() != 1) {
                _diagManager.error(
                    node->getLocation(),
                    "Character literal must be a single character"
                );
            }
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDLITERALCHECKER_HPP
