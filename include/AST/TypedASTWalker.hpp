#ifndef GLU_AST_TYPED_WALKER_HPP
#define GLU_AST_TYPED_WALKER_HPP

#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

namespace glu::ast {

/// @brief A kind of ASTVisitor that visits all nodes in the AST, accumulating
/// the results of the visit.
/// @tparam Impl the implementation class that inherits from this class.
/// @tparam ExprBaseRetTy the return type of the visit methods for ExprBase.
/// @tparam StmtBaseRetTy the return type of the visit methods for StmtBase.
/// @tparam DeclBaseRetTy the return type of the visit methods for DeclBase.
/// @details
/// Different methods can be overloaded for different traversal orders. The
/// methods are called in this order:
/// - beforeVisitNode(node) -- (which you can overload based on
/// ExprBase/StmtBase/DeclBase)
/// - preVisit<NodeKind>(node)
/// - (same for its child nodes)
/// - postVisit<NodeKind>(node, return for child 1, return for child 2, etc.)
/// - afterVisitNode(node) -- (which you can overload based on
/// ExprBase/StmtBase/DeclBase)
/// @note The return type of the visit methods is determined by the
/// template parameters. Those types must be default constructible, and thus
/// cannot be void. If no return type is needed, return an empty struct. The
/// default constructor is used when a node is missing (e.g. an empty else
/// branch in an if statement). It is also used as the default value for the
/// postVisit*Base functions, which can be overloaded to return a different
/// value.
/// @note The visit methods are not virtual, so they can be inlined. This is
/// important for performance, as the visitor is used in a lot of places.
template <
    typename Impl, typename ExprBaseRetTy, typename StmtBaseRetTy,
    typename DeclBaseRetTy>
class TypedASTWalker {
#define NODE_KIND(Name, Parent) using Name##RetTy = Parent##RetTy;
#define NODE_KIND_SUB_SUPER(Name, Parent) NODE_KIND(Name, Parent)
#include "NodeKind.def"
    using ASTNodeRetTy = void;

    ExprBaseRetTy _retTy(ExprBase *) { }
    StmtBaseRetTy _retTy(StmtBase *) { }
    DeclBaseRetTy _retTy(DeclBase *) { }

protected:
    Impl *asImpl() { return static_cast<Impl *>(this); }

    template <typename T> struct Callbacks {
        Impl *visitor;
        T *content;

        Callbacks(Impl *visitor, T *content)
            : visitor(visitor), content(content)
        {
            visitor->beforeVisitNode(content);
        }
        ~Callbacks() { visitor->afterVisitNode(content); }
    };

public:
    void preVisitASTNode(ASTNode *) { }
    void beforeVisitNode(ASTNode *) { }
    void afterVisitNode(ASTNode *) { }

    template <typename T> auto visit(T *node) -> decltype(_retTy(node))
    {
        Callbacks<T> callbacks(asImpl(), node);
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                                     \
case NodeKind::Name##Kind:                                          \
    if constexpr (std::is_same_v<                                   \
                      decltype(_retTy(node)),                       \
                      decltype(_visit##Name(nullptr))>) {           \
        return asImpl()->_visit##Name(llvm::cast<Name>(node));      \
    } else {                                                        \
        llvm_unreachable("Compile time and runtime type mismatch"); \
    }
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }

    template <typename T>
    auto visitArray(llvm::ArrayRef<T> nodes)
        -> llvm::SmallVector<decltype(_retTy(nodes[0]))>
    {
        llvm::SmallVector<decltype(_retTy(nodes[0]))> results;
        for (auto node : nodes) {
            results.push_back(visit(node));
        }
        return results;
    }

    // preVisit and postVisit functions
#define NODE_CHILD(Type, Name) [[maybe_unused]] Type##RetTy Name
#define NODE_TYPEREF(Type, Name) [[maybe_unused]] glu::types::Type *Name
#define NODE_CHILDREN(Type, Name)                     \
    [[maybe_unused]] llvm::ArrayRef<Type##RetTy> Name
#define NODE_KIND_(Name, Parent, ...)                                     \
    void preVisit##Name(Name *node) { asImpl()->preVisit##Parent(node); } \
    Name##RetTy postVisit##Name(Name *node __VA_OPT__(, ) __VA_ARGS__)    \
    {                                                                     \
        if constexpr (std::is_same_v<Name##RetTy, Parent##RetTy>) {       \
            return asImpl()->postVisit##Parent(node);                     \
        } else {                                                          \
            return Name##RetTy();                                         \
        }                                                                 \
    }
#define NODE_KIND_SUPER(Name, Parent) NODE_KIND_(Name, Parent)
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"

    // _visit function
#define NODE_CHILD(Type, Name)                                     \
    (node->get##Name() ? visit(node->get##Name()) : Type##RetTy())
#define NODE_TYPEREF(Type, Name) node->get##Name()
#define NODE_CHILDREN(Type, Name) visitArray(node->get##Name())
#define NODE_KIND_(Name, Parent, ...)                                      \
    Name##RetTy _visit##Name(Name *node)                                   \
    {                                                                      \
        asImpl()->preVisit##Name(node);                                    \
        return asImpl()->postVisit##Name(node __VA_OPT__(, ) __VA_ARGS__); \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_TYPED_WALKER_HPP
