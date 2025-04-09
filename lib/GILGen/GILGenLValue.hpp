#ifndef GLU_GILGEN_GILGENLVALUE_HPP
#define GLU_GILGEN_GILGENLVALUE_HPP

#include "ASTVisitor.hpp"
#include "Context.hpp"
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

}

#endif /* GLU_GILGEN_GILGENLVALUE_HPP */
