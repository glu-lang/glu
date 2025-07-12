#ifndef GLU_AST_CHILD_MODIFIER_VISITOR_HPP
#define GLU_AST_CHILD_MODIFIER_VISITOR_HPP

namespace glu::ast {

// Forward declarations
class ASTNode;

/// @brief Replace a child node in its parent node
/// @param oldExpr The expression to replace
/// @param newExpr The new expression to replace it with
void replaceChild(ast::ASTNode *oldNode, ast::ASTNode *newNode);


} // namespace glu::ast

#endif // GLU_AST_CHILD_MODIFIER_VISITOR_HPP
