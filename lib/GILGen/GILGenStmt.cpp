#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "GILGen.hpp"
#include "GILGenExpr.hpp"
#include "GILGenLValue.hpp"
#include "Scope.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenStmt : public ASTVisitor<GILGenStmt, void> {
    Context ctx;
    llvm::SmallVector<Scope> scopes;

    /// Generates GIL code for the given function.
    GILGenStmt(
        gil::Module *module, ast::FunctionDecl *decl,
        llvm::BumpPtrAllocator &arena
    )
        : ctx(module, decl, arena), scopes()
    {
        scopes.push_back(decl);
        // Add function arguments to scope
        auto &scope = getCurrentScope();
        auto *fnDecl = ctx.getASTFunction();
        auto *gilFn = ctx.getCurrentFunction();
        for (size_t i = 0; i < fnDecl->getParams().size(); ++i) {
            auto *paramDecl = fnDecl->getParams()[i];
            auto gilArg = gilFn->getEntryBlock()->getArgument(i);
            // FIXME: this is a temporary solution, we shouldn't need to
            // allocate memory for parameters, we should be able to use the
            // argument directly.
            auto *alloca
                = ctx.buildAlloca(ctx.translateType(paramDecl->getType()));
            ctx.buildStore(gilArg, alloca->getResult(0));
            scope.variables.insert({ paramDecl, alloca->getResult(0) });
        }
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

    GILGenExpr expr() { return GILGenExpr(ctx, getCurrentScope()); }

    GILGenLValue lvalue() { return GILGenLValue(ctx, getCurrentScope()); }

    void visitStmtBase([[maybe_unused]] StmtBase *stmt)
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

    void visitBreakStmt([[maybe_unused]] BreakStmt *stmt)
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope())
            scope = scope->parent;
        assert(scope && "Break statement outside of loop");
        ctx.buildBr(scope->breakDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitContinueStmt([[maybe_unused]] ContinueStmt *stmt)
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope())
            scope = scope->parent;
        assert(scope && "Continue statement outside of loop");
        ctx.buildBr(scope->continueDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitAssignStmt(AssignStmt *stmt)
    {
        auto rhs = expr().visit(stmt->getExprRight());
        auto lhs = lvalue().visit(stmt->getExprLeft());

        ctx.buildStore(rhs, lhs);
    }

    void visitIfStmt(IfStmt *stmt)
    {
        auto condValue = expr().visit(stmt->getCondition());
        auto *thenBB = ctx.buildBB("then");
        auto *elseBB = stmt->getElse() ? ctx.buildBB("else") : nullptr;
        auto *endBB = ctx.buildBB("end");

        if (elseBB) {
            ctx.buildCondBr(condValue, thenBB, elseBB);
        } else {
            ctx.buildCondBr(condValue, thenBB, endBB);
        }

        ctx.positionAtEnd(thenBB);
        visit(stmt->getBody());
        ctx.buildBr(endBB);

        if (elseBB) {
            ctx.positionAtEnd(elseBB);
            visit(stmt->getElse());
            ctx.buildBr(endBB);
        }

        ctx.positionAtEnd(endBB);
    }

    void visitWhileStmt(WhileStmt *stmt)
    {
        auto *condBB = ctx.buildBB("cond");
        auto *bodyBB = ctx.buildBB("body");
        auto *endBB = ctx.buildBB("end");

        ctx.buildBr(condBB);

        ctx.positionAtEnd(condBB);
        auto condValue = expr().visit(stmt->getCondition());
        ctx.buildCondBr(condValue, bodyBB, endBB);

        ctx.positionAtEnd(bodyBB);

        pushScope(Scope(stmt->getBody(), &getCurrentScope()));
        getCurrentScope().continueDestination = condBB;
        getCurrentScope().breakDestination = endBB;

        visitCompoundStmtNoScope(stmt->getBody());

        popScope();

        ctx.buildBr(condBB);

        ctx.positionAtEnd(endBB);
    }

    void visitReturnStmt([[maybe_unused]] ReturnStmt *stmt)
    {
        if (stmt->getReturnExpr()) {
            ctx.buildRet(expr().visit(stmt->getReturnExpr()));
        } else {
            ctx.buildRetVoid();
        }
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitExpressionStmt(ExpressionStmt *stmt)
    {
        expr().visit(stmt->getExpr());
    }

    void visitForStmt(ForStmt *stmt)
    {
        // TODO: We need sema to can implement this

        // auto *condBB = ctx.buildBB("cond");
        // auto *bodyBB = ctx.buildBB("body");
        // auto *endBB = ctx.buildBB("end");

        // ctx.buildBr(condBB);

        // ctx.positionAtEnd(condBB);
        // auto condValue = expr().visit(stmt->getRange());
        // ctx.buildCondBr(condValue, bodyBB, endBB);

        // ctx.positionAtEnd(bodyBB);

        // pushScope(Scope(stmt->getBody(), &getCurrentScope()));
        // getCurrentScope().continueDestination = condBB;
        // getCurrentScope().breakDestination = endBB;

        // visitCompoundStmtNoScope(stmt->getBody());

        // popScope();

        // ctx.buildBr(condBB);

        // ctx.positionAtEnd(endBB);
        assert(false && "For statement not implemented");
    }

    void visitDeclStmt(DeclStmt *stmt)
    {
        auto *varDecl = llvm::cast<ast::VarLetDecl>(stmt->getDecl());

        auto ptr = ctx.buildAlloca(ctx.translateType(varDecl->getType()));
        if (auto *value = varDecl->getValue()) {
            auto valueGIL = expr().visit(value);
            ctx.buildStore(valueGIL, ptr->getResult(0));
        }
        getCurrentScope().variables.insert({ varDecl, ptr->getResult(0) });
    }
};

gil::Function *GILGen::generateFunction(
    gil::Module *module, ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
{
    if (decl->getBody() == nullptr) {
        // If the function has no body, we skip it
        return nullptr;
    }
    return GILGenStmt(module, decl, arena).ctx.getCurrentFunction();
}

gil::Module *GILGen::generateModule(
    ast::ModuleDecl *moduleDecl, llvm::BumpPtrAllocator &arena
)
{
    auto gilModule = new (arena) gil::Module(moduleDecl->getName());

    // Generate GIL for all functions in the module
    for (auto decl : moduleDecl->getDecls()) {
        if (auto fn = llvm::dyn_cast<ast::FunctionDecl>(decl)) {
            if (fn->getBody() == nullptr) {
                // If the function has no body, we skip it
                continue;
            }
            generateFunction(gilModule, fn, arena);
        }
    }

    return gilModule;
}

} // namespace glu::gilgen
