#ifndef GLU_GILGEN_GILGENLVALUE_HPP
#define GLU_GILGEN_GILGENLVALUE_HPP

#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "Exprs.hpp"
#include "Scope.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenLValue : public ASTVisitor<GILGenLValue, gil::Value> {
    Context &ctx;
    Scope &scope;

    GILGenLValue(Context &ctx, Scope &scope) : ctx(ctx), scope(scope) { }

    gil::Value visitASTNode([[maybe_unused]] ASTNode *expr)
    {
        assert(false && "Unknown expression kind");
        return gil::Value::getEmptyKey();
    }

    gil::Value visitRefExpr(RefExpr *expr)
    {
        return scope
            .lookupVariable(llvm::cast<VarLetDecl *>(expr->getVariable()))
            .value();
    }

    gil::Value visitStructMemberExpr(StructMemberExpr *expr)
    {
        // TODO: implement this funciton
        return gil::Value::getEmptyKey();
    }
};

}

#endif /* GLU_GILGEN_GILGENLVALUE_HPP */
