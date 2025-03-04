#ifndef GLU_AST_WALKER_HPP
#define GLU_AST_WALKER_HPP

#include "ASTVisitor.hpp"

#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

template <typename Impl, typename RetTy = void, typename... ArgTys>
class ASTWalker : public ASTVisitor<Impl, RetTy, ArgTys...> {
public:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#define NODE_CHILD(Type, Name)                                               \
    node->get##Name()                                                        \
        ? (this->visit(node->get##Name(), std::forward<ArgTys>(args)...), 0) \
        : 0
#define NODE_TYPEREF(Type, Name) 0
#define NODE_CHILDREN(Type, Name)                          \
    0;                                                     \
    for (auto child : node->get##Name()) {                 \
        this->visit(child, std::forward<ArgTys>(args)...); \
    }                                                      \
    0
#define NODE_KIND_(Name, Parent, ...)              \
    RetTy _visit##Name(Name *node, ArgTys... args) \
    {                                              \
        __VA_ARGS__;                               \
        return this->asImpl()->visit##Name(        \
            node, std::forward<ArgTys>(args)...    \
        );                                         \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
#pragma clang diagnostic pop
};

} // namespace glu::ast

#endif // GLU_AST_WALKER_HPP
