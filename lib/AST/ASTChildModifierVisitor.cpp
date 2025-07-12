#include "ASTChildModifierVisitor.hpp"
#include "ASTVisitor.hpp"
#include "Basic/Tokens.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

class ASTChildModifierVisitor
    : public ASTVisitor<ASTChildModifierVisitor, void, ASTNode *, ASTNode *> {
public:
#define NODE_CHILD(Type, Name)                      \
    (void) 0;                                       \
    if (node->get##Name() == oldNode) {             \
        node->set##Name(newNode); \
    }                                               \
    (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)                      \
    (void) 0;                                          \
    auto childrens = node->get##Name();                \
    for (size_t i = 0; i < childrens.size(); ++i) {    \
        if (childrens[i] == oldNode) {                 \
            llvm::SmallVector<Type *, 4> newChildrens( \
                childrens.begin(), childrens.end()     \
            );                                         \
            newChildrens.push_back(newNode);           \
            node->set##Name(newChildrens);             \
            break;                                     \
        }                                              \
    }                                                  \
    (void) 0
#define NODE_KIND_SUPER(Name, Parent)
#define NODE_KIND_(Name, Parent, ...)                                \
    NODE_KIND_SUPER(Name, Parent)                                    \
    void visit##Name(Name *node, ASTNode *oldNode, ASTNode *newNode) \
    {                                                                \
        auto oldNode = llvm::cast<Type>(oldNode);                    \
        __VA_ARGS__;                                                 \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

void replaceChild(ast::ASTNode *oldNode, ast::ASTNode *newNode)
{
    ASTNode *parent = oldNode->getParent();

    if (parent == nullptr) {
        // Cannot replace root node
        return;
    }

    ASTChildModifierVisitor visitor;
    visitor.visit(parent, oldNode, newNode);
}

} // namespace glu::ast
