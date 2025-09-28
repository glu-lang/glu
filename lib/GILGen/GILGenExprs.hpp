#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"
#include "Scope.hpp"

#include "Exprs.hpp"

namespace glu::gilgen {

gil::Value visitExpr(Context &ctx, Scope &scope, ast::ExprBase *expr);
gil::Value visitLValue(Context &ctx, Scope &scope, ast::ExprBase *expr);

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
