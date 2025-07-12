#include "ASTChildModifierVisitor.hpp"
#include "ASTVisitor.hpp"
#include "Basic/Tokens.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

class ASTChildModifierVisitor : public ASTVisitor<ASTChildModifierVisitor> {
public:
#define NODE_CHILD(Type, Name)          \
    (void) 0;                           \
    if (node->get##Name() == oldExpr) { \
        node->set##Name(newExpr);       \
    }                                   \
    (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)                      \
    (void) 0;                                          \
    auto childrens = node->get##Name();                \
    for (size_t i = 0; i < childrens.size(); ++i) {    \
        if (childrens[i] == oldExpr) {                 \
            llvm::SmallVector<Type *, 8> newChildrens( \
                childrens.begin(), childrens.end()     \
            );                                         \
            newChildrens[i] = newExpr;                 \
            node->set##Name(newChildrens);             \
            break;                                     \
        }                                              \
    }                                                  \
    (void) 0
#define NODE_KIND_SUPER(Name, Parent) (void) 0
#define NODE_KIND_(Name, Parent, ...)                               \
    NODE_KIND_SUPER(Name, Parent)                                   \
    void visit##Name(Name *node, ArgTys... args) { __VA_ARGS__; }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

void replaceChildExpr(ExprBase *oldExpr, ExprBase *newExpr)
{
    ASTChildModifierVisitor visitor;
    visitor.visit(oldExpr, newExpr);
}

} // namespace glu::ast
