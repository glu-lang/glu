#ifndef GLU_SEMA_INVALID_OPERATOR_ARGS_HPP
#define GLU_SEMA_INVALID_OPERATOR_ARGS_HPP

#include "AST/ASTWalker.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

enum class OperatorType : char { Unary, Binary, UnaryAndBinary, Unknown };

class InvalidOperatorArgsChecker
    : public ast::ASTWalker<InvalidOperatorArgsChecker, void> {
    glu::DiagnosticManager &_diagManager;

public:
    InvalidOperatorArgsChecker(glu::DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void postVisitFunctionDecl(glu::ast::FunctionDecl *node)
    {
        auto name = node->getName();
        auto paramCount = node->getParamCount();

        auto opType = llvm::StringSwitch<OperatorType>(name)
#define OPERATOR(Name, value, type) .Case(value, OperatorType::type)
#include "Basic/TokenKind.def"
                          .Default(OperatorType::Unknown);
        switch (opType) {
        case OperatorType::Unary:
            if (paramCount != 1) {
                _diagManager.error(
                    node->getLocation(),
                    "Invalid argument count in unary operator '" + name + "'; "
                        + std::to_string(paramCount) + " provided"
                );
            }
            break;
        case OperatorType::Binary:
            if (paramCount != 2) {
                _diagManager.error(
                    node->getLocation(),
                    "Invalid argument count in binary operator '" + name + "'; "
                        + std::to_string(paramCount) + " provided"
                );
            }
            break;
        case OperatorType::UnaryAndBinary:
            if (paramCount != 1 && paramCount != 2) {
                _diagManager.error(
                    node->getLocation(),
                    "Invalid argument count in operator '" + name + "'; "
                        + std::to_string(paramCount) + " provided"
                );
            }
            break;
        case OperatorType::Unknown: break;
        }
    }
};
}

#endif // GLU_SEMA_INVALID_OPERATOR_ARGS_HPP
