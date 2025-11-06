#ifndef GLU_AST_EXPR_CALL_EXPR_HPP
#define GLU_AST_EXPR_CALL_EXPR_HPP

#include "ASTNode.hpp"
#include "ASTNodeMacros.hpp"
#include "Types/TypeBase.hpp"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/TrailingObjects.h>

namespace glu::ast {

/// @brief Represents a call expression in the AST (e.g., f(1, 2)).
class CallExpr final : public ExprBase,
                       private llvm::TrailingObjects<CallExpr, ExprBase *> {

    GLU_AST_GEN_CHILD(CallExpr, ExprBase *, _callee, Callee)
    GLU_AST_GEN_CHILDREN_TRAILING_OBJECTS(CallExpr, _argCount, ExprBase *, Args)

    llvm::SmallVector<glu::types::TypeBase *, 4> _templateArgs;

private:
    CallExpr(
        ExprBase *callee, llvm::ArrayRef<ExprBase *> args,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs, SourceLocation loc
    )
        : ExprBase(NodeKind::CallExprKind, loc)
        , _templateArgs(templateArgs.begin(), templateArgs.end())
    {
        initCallee(callee);
        initArgs(args);
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
        llvm::ArrayRef<ExprBase *> args,
        llvm::ArrayRef<glu::types::TypeBase *> templateArgs = {}
    )
    {
        void *mem = allocator.Allocate(
            totalSizeToAlloc<ExprBase *>(args.size()), alignof(CallExpr)
        );
        return new (mem) CallExpr(callee, args, templateArgs, loc);
    }

    static bool classof(ASTNode const *node)
    {
        return node->getKind() == NodeKind::CallExprKind;
    }

    llvm::ArrayRef<glu::types::TypeBase *> getTemplateArgs() const
    {
        return _templateArgs;
    }

    bool hasTemplateArgs() const { return !_templateArgs.empty(); }
};
}

#endif // GLU_AST_EXPR_CALL_EXPR_HPP
