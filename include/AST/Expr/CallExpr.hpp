#ifndef GLU_AST_EXPR_CALL_EXPR_HPP
#define GLU_AST_EXPR_CALL_EXPR_HPP

#include "ASTNode.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @brief Represents a call expression in the AST (e.g., f(1, 2)).
class CallExpr final : public ExprBase,
                       private llvm::TrailingObjects<CallExpr, ExprBase *> {
    using TrailingArgs = llvm::TrailingObjects<CallExpr, ExprBase *>;
    ExprBase *_callee;
    unsigned _argCount;
    friend TrailingArgs;

private:
    // Method required by llvm::TrailingObjects to determine the number
    // of trailing objects.
    size_t numTrailingObjects(typename TrailingArgs::OverloadToken<ExprBase *>)
        const
    {
        return _argCount;
    }

    CallExpr(
        ExprBase *callee, llvm::ArrayRef<ExprBase *> args, SourceLocation loc
    )
        : ExprBase(NodeKind::CallExprKind, loc)
        , _callee(callee)
        , _argCount(args.size())
    {
        std::uninitialized_copy(
            args.begin(), args.end(), getTrailingObjects<ExprBase *>()
        );
    }

public:
    /// @brief Constructs a CallExpr object. (to be called via the memory arena)
    /// @param allocator The allocator of the memory arena (internal)
    /// @param loc the location of the opening parenthesis
    /// @param callee the expression representing the function (pointer) to call
    /// @param args the arguments to pass to the function
    /// @return the newly created CallExpr object
    static CallExpr *create(
        llvm::BumpPtrAllocator &allocator, SourceLocation loc, ExprBase *callee,
        llvm::ArrayRef<ExprBase *> args
    )
    {
        void *mem = allocator.Allocate(
            sizeof(CallExpr) + sizeof(ExprBase *) * args.size(),
            alignof(CallExpr)
        );
        return new (mem) CallExpr(callee, args, loc);
    }

    /// @brief Returns the callee function of the call expression.
    ExprBase *getCallee() { return _callee; }

    /// @brief Returns the arguments of the call expression.
    llvm::ArrayRef<ExprBase *> getArgs() const
    {
        return { getTrailingObjects<ExprBase *>(), _argCount };
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::CallExprKind;
    }
};

}

#endif // GLU_AST_EXPR_CALL_EXPR_HPP
