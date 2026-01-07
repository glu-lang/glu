#ifndef GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
#define GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP

#include "AST/ASTWalker.hpp"
#include "Basic/Diagnostic.hpp"

namespace glu::sema {

/// @brief Resolves enum case values and ensures they have literal expressions.
class EnumValueResolver : public ast::ASTWalker<EnumValueResolver, void> {
    DiagnosticManager &_diagManager;

    /// @brief Gets the representable type for an enum, creating a default i32
    /// if none specified.
    /// @return The representable type (IntTy or CharTy).
    static types::TypeBase *
    getOrCreateRepresentableType(ast::EnumDecl *node, ast::ASTContext &ctx)
    {
        auto *repr = node->getRepresentableType();
        if (repr)
            return repr->getCanonicalType(ctx);
        return ctx.getTypesMemoryArena().create<types::IntTy>(
            types::IntTy::Signed, 32
        );
    }

    /// @brief Gets the bit width from a representable type.
    static unsigned getBitWidth(types::TypeBase *reprType)
    {
        if (auto *intTy = llvm::dyn_cast<types::IntTy>(reprType))
            return intTy->getBitWidth();
        if (llvm::isa<types::CharTy>(reprType))
            return 8;
        return 32;
    }

    static ast::LiteralExpr *makeLiteral(
        ast::ASTContext &ctx, llvm::APInt const &value,
        types::TypeBase *reprType, SourceLocation loc
    )
    {
        return ctx.getASTMemoryArena().create<ast::LiteralExpr>(
            value, reprType, loc
        );
    }

    bool
    tryGetLiteralValue(ast::ExprBase *expr, unsigned bitWidth, llvm::APInt &out)
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

        out = std::get<llvm::APInt>(literalValue).zextOrTrunc(bitWidth);
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
        auto *reprType = getOrCreateRepresentableType(node, *ctx);
        unsigned bitWidth = getBitWidth(reprType);
        bool hasValue = false;
        llvm::APInt current(bitWidth, 0);
        llvm::APInt explicitValue(bitWidth, 0);

        for (auto *field : node->getFields()) {
            auto *valueExpr = field->getValue();
            if (valueExpr
                && tryGetLiteralValue(valueExpr, bitWidth, explicitValue)) {
                field->setValue(makeLiteral(
                    *ctx, explicitValue, reprType, valueExpr->getLocation()
                ));
                current = explicitValue;
                hasValue = true;
                continue;
            }

            if (!hasValue) {
                current = llvm::APInt(bitWidth, 0);
                hasValue = true;
            } else {
                ++current;
            }
            field->setValue(
                makeLiteral(*ctx, current, reprType, field->getLocation())
            );
        }
    }
};

} // namespace glu::sema

#endif // GLU_SEMA_SEMANTICPASS_ENUMVALUERESOLVER_HPP
