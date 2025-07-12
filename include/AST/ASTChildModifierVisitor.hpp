#ifndef GLU_AST_CHILD_MODIFIER_VISITOR_HPP
#define GLU_AST_CHILD_MODIFIER_VISITOR_HPP

namespace glu::ast {

// Forward declarations
class ExprBase;

/// @brief Replace a child expression in its parent node
/// @param oldExpr The expression to replace
/// @param newExpr The new expression to replace it with
void replaceChildExpr(ExprBase *oldExpr, ExprBase *newExpr);

} // namespace glu::ast

#endif // GLU_AST_CHILD_MODIFIER_VISITOR_HPP
