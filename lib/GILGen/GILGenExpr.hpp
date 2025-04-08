#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"

#include "ASTVisitor.hpp"
#include "Exprs.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenExpr : public ASTVisitor<GILGenExpr, gil::Value> {
    Context &ctx;

    GILGenExpr(Context &ctx) : ctx(ctx) { }

    gil::Value visitExprBase([[maybe_unused]] ExprBase *expr)
    {
        assert(false && "Unknown expression kind");
        return gil::Value::getEmptyKey();
    }

    gil::Value visitAsLValue(ExprBase *expr)
    {
        switch (expr->getKind()) {
        case NodeKind::RefExprKind:
            return visitRefExprAsLValue(llvm::cast<RefExpr>(expr));
        case NodeKind::StructMemberExprKind:
            return visitStructMemberExprAsLValue(
                llvm::cast<StructMemberExpr>(expr)
            );
        default: assert(false && "Invalid expression for LValue");
        }
    }

    gil::Value visitRefExprAsLValue(RefExpr *expr)
    {
        // TODO: search the variable in the current scope
        return gil::Value::getEmptyKey();
    }

    gil::Value visitStructMemberExprAsLValue(StructMemberExpr *expr)
    {
        // gil::Value structValue = visitAsLValue(expr->getStructExpr());
        // TODO: return ctx.buildStructFieldPtr();
        return gil::Value::getEmptyKey();
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
