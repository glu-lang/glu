#ifndef GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
#define GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP

#include "AST/ASTWalker.hpp"
#include "Basic/Diagnostic.hpp"

#include <llvm/ADT/APSInt.h>

namespace glu::sema {

/// @brief Resolves enum case values and ensures they have literal expressions.
class EnumValueResolver : public ast::ASTWalker<EnumValueResolver, void> {
    DiagnosticManager &_diagManager;

    struct EnumReprInfo {
        unsigned bitWidth = 32;
        bool isUnsigned = false;
    };

    static EnumReprInfo getEnumReprInfo(ast::EnumDecl *node)
    {
        EnumReprInfo info;
        auto *repr = node->getRepresentableType();
        while (auto *alias = llvm::dyn_cast_or_null<types::TypeAliasTy>(repr)) {
            repr = alias->getWrappedType();
        }

        if (auto *intTy = llvm::dyn_cast_or_null<types::IntTy>(repr)) {
            info.bitWidth = intTy->getBitWidth();
            info.isUnsigned = intTy->isUnsigned();
        } else if (llvm::isa_and_nonnull<types::CharTy>(repr)) {
            info.bitWidth = 8;
            info.isUnsigned = false;
        }

        return info;
    }

    static ast::LiteralExpr *makeLiteral(
        ast::ASTContext &ctx, llvm::APSInt const &value, SourceLocation loc
    )
    {
        auto *litType
            = ctx.getTypesMemoryArena().create<types::TypeVariableTy>();
        return ctx.getASTMemoryArena().create<ast::LiteralExpr>(
            llvm::APInt(value), litType, loc
        );
    }

    bool tryGetLiteralValue(
        ast::ExprBase *expr, EnumReprInfo info, llvm::APSInt &out
    )
    {
        auto *literal = llvm::dyn_cast<ast::LiteralExpr>(expr);
        if (!literal) {
            _diagManager.error(
                expr->getLocation(),
                "enum case value must be an integer literal"
            );
            return false;
        }

        auto literalValue = literal->getValue();
        if (!std::holds_alternative<llvm::APInt>(literalValue)) {
            _diagManager.error(
                expr->getLocation(),
                "enum case value must be an integer literal"
            );
            return false;
        }

        out = llvm::APSInt(
            std::get<llvm::APInt>(literalValue), info.isUnsigned
        );
        out = out.extOrTrunc(info.bitWidth);
        return true;
    }

public:
    explicit EnumValueResolver(DiagnosticManager &diagManager)
        : _diagManager(diagManager)
    {
    }

    void preVisitEnumDecl(ast::EnumDecl *node)
    {
        auto *ctx = node->getModule()->getContext();
        auto info = getEnumReprInfo(node);
        bool hasValue = false;
        llvm::APSInt current(info.bitWidth, info.isUnsigned);
        llvm::APSInt explicitValue(info.bitWidth, info.isUnsigned);

        for (auto *field : node->getFields()) {
            auto *valueExpr = field->getValue();
            if (valueExpr
                && tryGetLiteralValue(valueExpr, info, explicitValue)) {
                field->setValue(
                    makeLiteral(*ctx, explicitValue, valueExpr->getLocation())
                );
                current = explicitValue;
                hasValue = true;
                continue;
            }

            if (!hasValue) {
                current = llvm::APSInt(info.bitWidth, info.isUnsigned);
                hasValue = true;
            } else {
                current += llvm::APSInt(
                    llvm::APInt(info.bitWidth, 1), info.isUnsigned
                );
            }
            field->setValue(makeLiteral(*ctx, current, field->getLocation()));
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
