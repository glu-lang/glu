#ifndef GLU_AST_VISITOR_HPP
#define GLU_AST_VISITOR_HPP

#include "ASTNode.hpp"
#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <utility>

namespace glu::ast {

template <typename Impl, typename RetTy = void, typename... ArgTys>
class ASTVisitor {
    Impl *asImpl() { return static_cast<Impl *>(this); }

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
        if (!node) {
            return asImpl()->visitASTNode(
                nullptr, std::forward<ArgTys>(args)...
            );
        }
        return _visitAST(node, std::forward<ArgTys>(args)...);
    }

    RetTy _visitAST(ASTNode *node, ArgTys... args)
    {
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                               \
case NodeKind::Name##Kind:                                    \
    return asImpl()->visit##Name(                             \
        llvm::cast<Name>(node), std::forward<ArgTys>(args)... \
    );
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }

    RetTy visitASTNode(ASTNode *node, ArgTys... args)
    {
        if constexpr (std::is_default_constructible_v<RetTy>) {
            return RetTy();
        } else {
            assert(
                false && "No default implementation provided for this Node."
            );
        }
    }

#define NODE_KIND(Name, Parent)                                              \
    RetTy visit##Name(Name *node, ArgTys... args)                            \
    {                                                                        \
        return asImpl()->visit##Parent(node, std::forward<ArgTys>(args)...); \
    }
#define NODE_KIND_SUPER(Name, Parent) NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_VISITOR_HPP
