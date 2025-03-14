#ifndef GLU_AST_WALKER_HPP
#define GLU_AST_WALKER_HPP

#include "ASTVisitor.hpp"

#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

#include <llvm/ADT/ScopeExit.h>

namespace glu::ast {

enum class TraversalOrder {
    PreOrder,
    PostOrder,
};

template <
    typename Impl, TraversalOrder Order = TraversalOrder::PostOrder,
    typename RetTy = void, typename... ArgTys>
class ASTWalker : public ASTVisitor<Impl, RetTy, ArgTys...> {
public:
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
#define NODE_KIND_(Name, Parent, ...)                          \
    RetTy _visit##Name(Name *node, ArgTys... args)             \
    {                                                          \
        auto defer = llvm::make_scope_exit([&] {               \
            if constexpr (Order == TraversalOrder::PreOrder) { \
                __VA_ARGS__;                                   \
            }                                                  \
        });                                                    \
        if constexpr (Order == TraversalOrder::PostOrder) {    \
            __VA_ARGS__;                                       \
        }                                                      \
        return this->asImpl()->visit##Name(                    \
            node, std::forward<ArgTys>(args)...                \
        );                                                     \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_WALKER_HPP
