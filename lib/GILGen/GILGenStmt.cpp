#ifndef GLU_GILGEN_GILGENSTMT_HPP
#define GLU_GILGEN_GILGENSTMT_HPP

#include "Context.hpp"
#include "Scope.hpp"

#include "ASTVisitor.hpp"
#include "GILGen.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenStmt : public ASTVisitor<GILGenStmt, void> {
    Context ctx;
    llvm::SmallVector<Scope> scopes;

    /// Generates GIL code for the given function.
    GILGenStmt(ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena)
        : ctx(decl, arena), scopes()
    {
        scopes.push_back(decl);
        visitCompoundStmtNoScope(ctx.getASTFunction()->getBody());
        // at the end of the function, return void (error if function is not
        // void)
        if (llvm::isa<types::VoidTy>(decl->getType()->getReturnType())) {
            ctx.buildRetVoid();
        } else {
            // TODO: If reachable, error missing return
            ctx.buildUnreachable();
        }
    }

    Scope &getCurrentScope() { return scopes.back(); }
    void pushScope(Scope &&scope) { scopes.push_back(std::move(scope)); }
    void popScope() { scopes.pop_back(); }

    void visitStmtBase(StmtBase *stmt)
    {
        assert(false && "Unknown statement kind");
    }

    void visitCompoundStmt(CompoundStmt *stmt)
    {
        pushScope(Scope(stmt, &getCurrentScope()));
        visitCompoundStmtNoScope(stmt);
        popScope();
    }

    void visitCompoundStmtNoScope(CompoundStmt *stmt)
    {
        for (StmtBase *child : stmt->getStmts())
            visit(child);
    }

    void visitBreakStmt(BreakStmt *stmt)
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope())
            scope = scope->parent;
        assert(scope && "Break statement outside of loop");
        ctx.buildBr(scope->breakDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitContinueStmt(ContinueStmt *stmt)
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope())
            scope = scope->parent;
        assert(scope && "Continue statement outside of loop");
        ctx.buildBr(scope->continueDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }
};

gil::Function *
GILGen::generateFunction(ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena)
{
    return GILGenStmt(decl, arena).ctx.getCurrentFunction();
}

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENSTMT_HPP
