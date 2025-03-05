#ifndef GLU_AST_VISITOR_HPP
#define GLU_AST_VISITOR_HPP

#include "ASTNode.hpp"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <utility>

namespace glu::ast {

#define NODE_KIND(Name, Parent) class Name;
#define NODE_KIND_SUPER(Name, Parent) class Name;
#include "NodeKind.def"

template <typename Impl, typename RetTy = void, typename... ArgTys>
class ASTVisitor {
protected:
    Impl *asImpl() { return static_cast<Impl *>(this); }

    // TODO: pass the arguments (ArgTys) to callbacks if needed
    struct Callbacks {
        Impl *visitor;
        ASTNode *content;

        Callbacks(Impl *visitor, ASTNode *content)
            : visitor(visitor), content(content)
        {
            visitor->beforeVisitNode(content);
        }
        ~Callbacks() { visitor->afterVisitNode(content); }
    };

public:
    /**
     * @brief Visit an AST node.
     *
     * This function call the appropriate visit function for the node's kind.
     *
     * @param node The node to visit.
     * @param ...args The arguments to pass to the visit function.
     * @return The value returned by the visit function.
     *
     **/
    RetTy visit(ASTNode *node, ArgTys... args)
    {
        Callbacks callbacks(asImpl(), node);
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                               \
case NodeKind::Name##Kind:                                    \
    return asImpl()->_visit##Name(                            \
        llvm::cast<Name>(node), std::forward<ArgTys>(args)... \
    );
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }

    /// @brief An action to run before visiting a node.
    /// @param node the node about to be visited
    void beforeVisitNode([[maybe_unused]] ASTNode *node) { }
    /// @brief An action to run after visiting a node.
    /// @param node the node that was just visited
    void afterVisitNode([[maybe_unused]] ASTNode *node) { }

    RetTy visitASTNode(
        [[maybe_unused]] ASTNode *node, [[maybe_unused]] ArgTys... args
    )
    {
    }

#define NODE_KIND(Name, Parent)                                              \
    RetTy visit##Name(Name *node, ArgTys... args)                            \
    {                                                                        \
        return asImpl()->visit##Parent(node, std::forward<ArgTys>(args)...); \
    }                                                                        \
    RetTy _visit##Name(Name *node, ArgTys... args)                           \
    {                                                                        \
        return asImpl()->visit##Name(node, std::forward<ArgTys>(args)...);   \
    }
#define NODE_KIND_SUPER(Name, Parent) NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_VISITOR_HPP
