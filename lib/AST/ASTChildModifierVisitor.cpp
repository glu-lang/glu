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
    if (node->get##Name() == oldNode) { \
        node->set##Name(newNode);       \
    }                                   \
    (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)                      \
    (void) 0;                                          \
    auto childrens = node->get##Name();                \
    for (size_t i = 0; i < childrens.size(); ++i) {    \
        if (childrens[i] == oldNode) {                 \
            llvm::SmallVector<Type *, 8> newChildrens( \
                childrens.begin(), childrens.end()     \
            );                                         \
            newChildrens[i] = newNode;                 \
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

void replaceChild(ast::ASTNode *oldNode, ast::ASTNode *newNode)
{
    ASTChildModifierVisitor visitor;
    visitor.visit(oldNode, newNode);
}

} // namespace glu::ast
