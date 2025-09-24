#ifndef GLU_SEMA_SEMANTICPASS_VALIDMAINCHECKER_HPP
#define GLU_SEMA_SEMANTICPASS_VALIDMAINCHECKER_HPP

#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/ASTWalker.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Validates main function signatures to ensure they follow C-like
/// conventions. Only allows the following signatures:
/// - func main() -> Void or Int
/// - func main(argc: Int, argv: **Char) -> Void or Int
/// - func main(argc: Int, argv: **Char, envp: **Char) -> Void or Int
class ValidMainChecker : public ast::ASTWalker<ValidMainChecker, void> {
    DiagnosticManager &_diagManager;

public:
    explicit ValidMainChecker(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitFunctionDecl(ast::FunctionDecl *node)
    {
        // Only check functions named "main"
        if (node->getName() != "main") {
            return;
        }

        auto *funcType = node->getType();
        auto returnType = funcType->getReturnType();
        auto params = funcType->getParameters();

        // Check return type: must be Void or Int
        if (!isValidMainReturnType(returnType)) {
            _diagManager.error(
                node->getLocation(), "main function must return Void or Int"
            );
            return;
        }

        // Check parameter signatures
        if (!isValidMainSignature(params, node->getLocation())) {
            return; // Error already reported in isValidMainSignature
        }
    }

private:
    /// @brief Checks if the return type is valid for main function (Void or
    /// Int)
    bool isValidMainReturnType(glu::types::TypeBase *returnType) const
    {
        return llvm::isa<glu::types::VoidTy>(returnType)
            || llvm::isa<glu::types::IntTy>(returnType);
    }

    /// @brief Checks if the parameter signature is valid for main function
    bool isValidMainSignature(
        llvm::ArrayRef<glu::types::TypeBase *> params,
        glu::SourceLocation location
    )
    {
        size_t paramCount = params.size();

        switch (paramCount) {
        case 0:
            // func main() -> Void or Int
            return true;

        case 2:
            // func main(argc: Int, argv: **Char) -> Void or Int
            return validateTwoParamMain(params, location);

        case 3:
            // func main(argc: Int, argv: **Char, envp: **Char) -> Void or Int
            return validateThreeParamMain(params, location);

        default:
            _diagManager.error(
                location,
                llvm::Twine(
                    "main function must have 0, 2, or 3 parameters, got "
                ) + llvm::Twine(paramCount)
            );
            return false;
        }
    }

    /// @brief Validates two-parameter main signature: main(argc: Int, argv:
    /// **Char)
    bool validateTwoParamMain(
        llvm::ArrayRef<glu::types::TypeBase *> params,
        glu::SourceLocation location
    )
    {
        // First parameter (argc) must be Int
        if (!llvm::isa<glu::types::IntTy>(params[0])) {
            _diagManager.error(
                location,
                "first parameter of main function (argc) must be of type Int"
            );
            return false;
        }

        // Second parameter (argv) must be **Char (pointer to pointer to Char)
        if (!isCharPointerPointer(params[1])) {
            _diagManager.error(
                location,
                "second parameter of main function (argv) must be of type "
                "**Char"
            );
            return false;
        }

        return true;
    }

    /// @brief Validates three-parameter main signature: main(argc: Int, argv:
    /// **Char, envp: **Char)
    bool validateThreeParamMain(
        llvm::ArrayRef<glu::types::TypeBase *> params,
        glu::SourceLocation location
    )
    {
        // Validate first two parameters same as two-param version
        if (!validateTwoParamMain(params.take_front(2), location)) {
            return false;
        }

        // Third parameter (envp) must be **Char (pointer to pointer to Char)
        if (!isCharPointerPointer(params[2])) {
            _diagManager.error(
                location,
                "third parameter of main function (envp) must be of type **Char"
            );
            return false;
        }

        return true;
    }

    /// @brief Checks if a type is **Char (pointer to pointer to Char)
    bool isCharPointerPointer(glu::types::TypeBase *type) const
    {
        // Must be PointerTy
        auto *outerPtr = llvm::dyn_cast<glu::types::PointerTy>(type);
        if (!outerPtr) {
            return false;
        }

        // The pointee must also be PointerTy
        auto *innerPtr
            = llvm::dyn_cast<glu::types::PointerTy>(outerPtr->getPointee());
        if (!innerPtr) {
            return false;
        }

        // The final pointee must be CharTy
        return llvm::isa<glu::types::CharTy>(innerPtr->getPointee());
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_VALIDMAINCHECKER_HPP
