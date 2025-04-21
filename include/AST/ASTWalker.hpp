#ifndef GLU_AST_WALKER_HPP
#define GLU_AST_WALKER_HPP

#include "ASTVisitor.hpp"

#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

#include <llvm/ADT/ScopeExit.h>

namespace glu::ast {

/// @brief A kind of ASTVisitor that visits all nodes in the AST.
/// @tparam Impl the implementation class that inherits from this class.
/// @tparam RetTy the return type of the visit methods.
/// @tparam ...ArgTys the argument types of the visit methods.
/// @details
/// Different methods can be overloaded for different traversal orders. The
/// methods are called in this order:
/// - beforeVisitNode
/// - preVisit<NodeKind> -----------]
/// - (same for its child nodes) ---] _visit<NodeKind> overrides this behaviour
/// - visit<NodeKind> --------------]
/// - afterVisitNode
template <typename Impl, typename RetTy = void, typename... ArgTys>
class ASTWalker : public ASTVisitor<Impl, RetTy, ArgTys...> {
public:
    void preVisitASTNode(
        [[maybe_unused]] ASTNode *node, [[maybe_unused]] ArgTys... args
    ) {}
#define NODE_CHILD(Type, Name)                                             \
    (node->get##Name()                                                     \
         ? (this->visit(node->get##Name(), std::forward<ArgTys>(args)...)) \
         : (void) 0)
#define NODE_TYPEREF(Type, Name) (void) 0
#define NODE_CHILDREN(Type, Name)                          \
    (void) 0;                                              \
    for (auto child : node->get##Name()) {                 \
        this->visit(child, std::forward<ArgTys>(args)...); \
    }                                                      \
    (void) 0
#define NODE_KIND_SUPER(Name, Parent)                                          \
    void preVisit##Name(Name *node, ArgTys... args)                            \
    {                                                                          \
        this->asImpl()->preVisit##Parent(node, std::forward<ArgTys>(args)...); \
    }
#define NODE_KIND_(Name, Parent, ...)                                        \
    NODE_KIND_SUPER(Name, Parent)                                            \
    RetTy _visit##Name(Name *node, ArgTys... args)                           \
    {                                                                        \
        this->asImpl()->preVisit##Name(node, std::forward<ArgTys>(args)...); \
        __VA_ARGS__;                                                         \
        return this->asImpl()->visit##Name(                                  \
            node, std::forward<ArgTys>(args)...                              \
        );                                                                   \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_WALKER_HPP
