#ifndef GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
#define GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP

#include "AST/ASTWalker.hpp"
#include "Basic/Diagnostic.hpp"

#include <llvm/ADT/APSInt.h>

namespace glu::sema {

/// @brief Resolves enum case values and ensures they have literal expressions.
class EnumValueResolver : public ast::ASTWalker<EnumValueResolver, void> {
    DiagnosticManager &_diagManager;

    static unsigned getEnumBitWidth(ast::EnumDecl *node)
    {
        auto *repr = node->getRepresentableType();
        if (!repr)
            return 32;

        auto *canonical
            = repr->getCanonicalType(*node->getModule()->getContext());

        if (auto *intTy = llvm::dyn_cast_or_null<types::IntTy>(canonical)) {
            return intTy->getBitWidth();
        } else if (llvm::isa_and_nonnull<types::CharTy>(canonical)) {
            return 8;
        }

        return 32;
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
        ast::ExprBase *expr, unsigned bitWidth, llvm::APSInt &out
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

        out = llvm::APSInt(std::get<llvm::APInt>(literalValue), false);
        out = out.extOrTrunc(bitWidth);
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
        unsigned bitWidth = getEnumBitWidth(node);
        bool hasValue = false;
        llvm::APSInt current(bitWidth, false);
        llvm::APSInt explicitValue(bitWidth, false);

        for (auto *field : node->getFields()) {
            auto *valueExpr = field->getValue();
            if (valueExpr
                && tryGetLiteralValue(valueExpr, bitWidth, explicitValue)) {
                field->setValue(
                    makeLiteral(*ctx, explicitValue, valueExpr->getLocation())
                );
                current = explicitValue;
                hasValue = true;
                continue;
            }

            if (!hasValue) {
                current = llvm::APSInt(bitWidth, false);
                hasValue = true;
            } else {
                current += llvm::APSInt(llvm::APInt(bitWidth, 1), false);
            }
            field->setValue(makeLiteral(*ctx, current, field->getLocation()));
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
