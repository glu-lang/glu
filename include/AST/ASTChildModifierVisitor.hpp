#ifndef GLU_AST_CHILD_MODIFIER_VISITOR_HPP
#define GLU_AST_CHILD_MODIFIER_VISITOR_HPP

#include "ASTVisitor.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {
template <typename... Args> void modifyChildren(ASTNode *node, Args &&...args);

} // namespace glu::ast

#endif // GLU_AST_CHILD_MODIFIER_VISITOR_HPP
