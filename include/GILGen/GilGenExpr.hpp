#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"

#include "ASTVisitor.hpp"
#include "Exprs.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GilGenExpr : public ASTVisitor<GilGenExpr, void> {
    Context &ctx;

    GilGenExpr(Context &ctx) : ctx(ctx) { }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
