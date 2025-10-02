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
/// Also ensures there is at most one main function in the module.
class ValidMainChecker : public ast::ASTWalker<ValidMainChecker, void> {
    DiagnosticManager &_diagManager;
    ast::FunctionDecl *_firstMainFunction = nullptr;

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

        // Check for multiple main functions
        if (_firstMainFunction == nullptr) {
            _firstMainFunction = node;
        } else {
            _diagManager.error(
                node->getLocation(),
                "multiple definitions of main function found",
                std::make_unique<Diagnostic>(
                    DiagnosticSeverity::Note, _firstMainFunction->getLocation(),
                    "first definition of main function here"
                )
            );
        }

        auto *funcType = node->getType();
        auto params = node->getParams();
        auto returnType = funcType->getReturnType();

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
        llvm::ArrayRef<glu::ast::ParamDecl *> params,
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
        llvm::ArrayRef<glu::ast::ParamDecl *> params,
        [[maybe_unused]] glu::SourceLocation location
    )
    {
        auto result = true;

        // First parameter (argc) must be Int
        if (!llvm::isa<glu::types::IntTy>(params[0]->getType())) {
            _diagManager.error(
                params[0]->getLocation(),
                "first parameter of main function must be of type Int"
            );
            result = false;
        }

        // Second parameter (argv) must be **Char (pointer to pointer to Char)
        if (!isCharPointerPointer(params[1]->getType())) {
            _diagManager.error(
                params[1]->getLocation(),
                "second parameter of main function must be of type "
                "**Char"
            );
            result = false;
        }

        return result;
    }

    /// @brief Validates three-parameter main signature: main(argc: Int, argv:
    /// **Char, envp: **Char)
    bool validateThreeParamMain(
        llvm::ArrayRef<glu::ast::ParamDecl *> params,
        glu::SourceLocation location
    )
    {
        auto result = true;

        // Validate first two parameters same as two-param version
        if (!validateTwoParamMain(params.take_front(2), location)) {
            result = false;
        }

        // Third parameter (envp) must be **Char (pointer to pointer to Char)
        if (!isCharPointerPointer(params[2]->getType())) {
            _diagManager.error(
                params[2]->getLocation(),
                "third parameter of main function must be of type **Char"
            );
            result = false;
        }

        return result;
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
