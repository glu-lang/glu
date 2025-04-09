#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"

#include "ASTVisitor.hpp"
#include "Exprs.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenLValue : public ASTVisitor<GILGenLValue, gil::Value> {
    Context &ctx;

    GILGenLValue(Context &ctx) : ctx(ctx) { }

    gil::Value visitExprBase([[maybe_unused]] ExprBase *expr)
    {
        assert(false && "Unknown expression kind");
        return gil::Value::getEmptyKey();
    }

    gil::Value visitRefExpr(RefExpr *expr)
    {
        // TODO: implement this function
        return gil::Value::getEmptyKey();
    }

    gil::Value visitStructMemberExpr(StructMemberExpr *expr)
    {
        // TODO: implement this funciton
        return gil::Value::getEmptyKey();
    }
};

struct GILGenExpr : public ASTVisitor<GILGenExpr, gil::Value> {
    Context &ctx;

    GILGenExpr(Context &ctx) : ctx(ctx) { }

    gil::Value visitExprBase([[maybe_unused]] ExprBase *expr)
    {
        assert(false && "Unknown expression kind");
        return gil::Value::getEmptyKey();
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
