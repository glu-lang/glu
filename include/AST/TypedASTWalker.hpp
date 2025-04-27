#ifndef GLU_AST_TYPED_WALKER_HPP
#define GLU_AST_TYPED_WALKER_HPP

#include "ASTVisitor.hpp"

#include "Decls.hpp"
#include "Exprs.hpp"
#include "Stmts.hpp"

#include <llvm/ADT/ScopeExit.h>

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
template <
    typename Impl, typename ExprBaseRetTy, typename StmtBaseRetTy,
    typename DeclBaseRetTy>
class TypedASTWalker {
#define NODE_KIND(Name, Parent) using Name##RetTy = Parent##RetTy;
#define NODE_KIND_SUB_SUPER(Name, Parent) NODE_KIND(Name, Parent)
#include "NodeKind.def"
    using ASTNodeRetTy = void;

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
    void preVisitASTNode([[maybe_unused]] ASTNode *node) { }
    void beforeVisitNode([[maybe_unused]] ASTNode *node) { }
    void afterVisitNode([[maybe_unused]] ASTNode *node) { }

    ExprBaseRetTy visit(ExprBase *node)
    {
        Callbacks<ExprBase> callbacks(asImpl(), node);
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                                          \
case NodeKind::Name##Kind:                                               \
    if constexpr (std::is_same_v<                                        \
                      ExprBaseRetTy, decltype(_visit##Name(nullptr))>) { \
        return asImpl()->_visit##Name(llvm::cast<Name>(node));           \
    } else {                                                             \
        llvm_unreachable("ExprBase is not an expr?");                    \
    }
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }
    StmtBaseRetTy visit(StmtBase *node)
    {
        Callbacks<StmtBase> callbacks(asImpl(), node);
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                                          \
case NodeKind::Name##Kind:                                               \
    if constexpr (std::is_same_v<                                        \
                      StmtBaseRetTy, decltype(_visit##Name(nullptr))>) { \
        return asImpl()->_visit##Name(llvm::cast<Name>(node));           \
    } else {                                                             \
        llvm_unreachable("StmtBase is not a stmt?");                     \
    }
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }
    DeclBaseRetTy visit(DeclBase *node)
    {
        Callbacks<DeclBase> callbacks(asImpl(), node);
        switch (node->getKind()) {
#define NODE_KIND(Name, Parent)                                          \
case NodeKind::Name##Kind:                                               \
    if constexpr (std::is_same_v<                                        \
                      DeclBaseRetTy, decltype(_visit##Name(nullptr))>) { \
        return asImpl()->_visit##Name(llvm::cast<Name>(node));           \
    } else {                                                             \
        llvm_unreachable("DeclBase is not a decl?");                     \
    }
#include "NodeKind.def"
        default: llvm_unreachable("Unknown node kind.");
        }
    }
    template <typename T>
    auto visitArray(llvm::ArrayRef<T> nodes)
        -> llvm::SmallVector<decltype(this->visit(nodes[0]))>
    {
        llvm::SmallVector<decltype(this->visit(nodes[0]))> results;
        for (auto node : nodes) {
            results.push_back(this->visit(node));
        }
        return results;
    }

    // preVisit and postVisit functions
#define NODE_CHILD(Type, Name) [[maybe_unused]] Type##RetTy Name
#define NODE_TYPEREF(Type, Name) [[maybe_unused]] glu::types::Type *Name
#define NODE_CHILDREN(Type, Name)                     \
    [[maybe_unused]] llvm::ArrayRef<Type##RetTy> Name
#define NODE_KIND_(Name, Parent, ...)                                  \
    void preVisit##Name(Name *node)                                    \
    {                                                                  \
        this->asImpl()->preVisit##Parent(node);                        \
    }                                                                  \
    Name##RetTy postVisit##Name(Name *node __VA_OPT__(, ) __VA_ARGS__) \
    {                                                                  \
        if constexpr (std::is_same_v<Name##RetTy, Parent##RetTy>) {    \
            return this->asImpl()->postVisit##Parent(node);            \
        } else {                                                       \
            return Name##RetTy();                                      \
        }                                                              \
    }
#define NODE_KIND_SUPER(Name, Parent) NODE_KIND_(Name, Parent)
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"

    // _visit function
#define NODE_CHILD(Type, Name)                                           \
    (node->get##Name() ? this->visit(node->get##Name()) : Type##RetTy())
#define NODE_TYPEREF(Type, Name) node->get##Name()
#define NODE_CHILDREN(Type, Name) visitArray(node->get##Name())
#define NODE_KIND_(Name, Parent, ...)                                          \
    Name##RetTy _visit##Name(Name *node)                                       \
    {                                                                          \
        this->asImpl()->preVisit##Name(node);                                  \
        return this->asImpl()->postVisit##Name(node __VA_OPT__(, ) __VA_ARGS__ \
        );                                                                     \
    }
#define NODE_KIND(Name, Parent)
#include "NodeKind.def"
};

} // namespace glu::ast

#endif // GLU_AST_TYPED_WALKER_HPP
