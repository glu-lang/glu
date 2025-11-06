#include "ASTVisitor.hpp"

namespace glu::ast {

class ASTChildReplacerVisitor
    : public ASTVisitor<ASTChildReplacerVisitor, void, ASTNode *, ASTNode *> {
public:
#define NODE_CHILD(Type, Name)                      \
    (void) 0;                                       \
    if (node->get##Name() == oldNode) {             \
        node->set##Name(llvm::cast<Type>(newNode)); \
    }                                               \
    (void) 0
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)                            \
    (void) 0;                                                \
    auto Name##Children = node->get##Name();                 \
    for (size_t i = 0; i < Name##Children.size(); ++i) {     \
        if (Name##Children[i] == oldNode) {                  \
            llvm::SmallVector<Type *, 8> newChildren(        \
                Name##Children.begin(), Name##Children.end() \
            );                                               \
            newChildren[i] = llvm::cast<Type>(newNode);      \
            node->set##Name(newChildren);                    \
            break;                                           \
        }                                                    \
    }                                                        \
    (void) 0

#define NODE_KIND_SUPER(Name, Parent)

#define NODE_KIND_(Name, Parent, ...)                                   \
    void visit##Name(                                                   \
        [[maybe_unused]] Name *node, [[maybe_unused]] ASTNode *oldNode, \
        [[maybe_unused]] ASTNode *newNode                               \
    )                                                                   \
    {                                                                   \
        __VA_ARGS__;                                                    \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

void replaceChild(ASTNode *parent, ASTNode *oldNode, ASTNode *newNode)
{
    if (parent == nullptr) {
        // Cannot replace root node
        return;
    }

    ASTChildReplacerVisitor visitor;
    visitor.visit(parent, oldNode, newNode);
}

} // namespace glu::ast
