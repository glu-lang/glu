#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "GILGen.hpp"
#include "GILGenExprs.hpp"
#include "Scope.hpp"

#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenStmt : public ASTVisitor<GILGenStmt, void> {
    Context ctx;
    // FIXME: On reallocations this will CRASH! More than 32 scopes will
    // cause issues.
    llvm::SmallVector<Scope, 32> scopes;

    /// Generates GIL code for the given function.
    GILGenStmt(
        gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
    )
        : ctx(module, decl, globalCtx)
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

    /// Generates GIL code for the given global initializer.
    GILGenStmt(
        gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
    )
        : ctx(module, decl, globalCtx)
    {
        scopes.push_back(nullptr);
        ctx.buildRet(expr(decl->getValue()));
    }

    Scope &getCurrentScope() { return scopes.back(); }
    void pushScope(Scope &&scope) { scopes.push_back(std::move(scope)); }
    void popScope()
    {
        auto &lastScope = scopes.back();
        dropScopeVariables(lastScope);
        scopes.pop_back();
    }

    gil::Value expr(ast::ExprBase *expr)
    {
        return visitExpr(ctx, getCurrentScope(), expr);
    }
    gil::Value lvalue(ast::ExprBase *expr)
    {
        return visitLValue(ctx, getCurrentScope(), expr);
    }

    void beforeVisitNode(ASTNode *node) { ctx.setSourceLocNode(node); }

    void afterVisitNode(ASTNode *node)
    {
        ctx.setSourceLocNode(node->getParent());
    }

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
        Scope *scope = dropLoopScopes();
        ctx.buildBr(scope->breakDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitContinueStmt([[maybe_unused]] ContinueStmt *stmt)
    {
        Scope *scope = dropLoopScopes();
        ctx.buildBr(scope->continueDestination);
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitAssignStmt(AssignStmt *stmt)
    {
        auto rhs = expr(stmt->getExprRight());
        auto lhs = lvalue(stmt->getExprLeft());

        ctx.buildStore(rhs, lhs);
    }

    void visitIfStmt(IfStmt *stmt)
    {
        auto condValue = expr(stmt->getCondition());
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
        auto condValue = expr(stmt->getCondition());
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
        if (auto *returnExpr = stmt->getReturnExpr()) {
            auto value = expr(returnExpr);
            dropFuncScope();
            ctx.buildRet(value);
        } else {
            dropFuncScope();
            ctx.buildRetVoid();
        }

        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitExpressionStmt(ExpressionStmt *stmt)
    {
        auto value = expr(stmt->getExpr());

        if (value == gil::Value::getEmptyKey()) {
            return;
        }

        ctx.buildDrop(value);
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
            auto valueGIL = expr(value);
            ctx.buildStore(valueGIL, ptr->getResult(0));
        }
        getCurrentScope().variables.insert({ varDecl, ptr->getResult(0) });
    }

private:
    void dropScopeVariables(Scope &scope)
    {
        for (auto &[var, val] : scope.variables) {
            ctx.buildDrop(val);
        }
    }

    /// Find the enclosing loop scope, dropping variables from intermediate
    /// scopes
    Scope *dropLoopScopes()
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope()) {
            dropScopeVariables(*scope);
            scope = scope->parent;
        }
        assert(scope && "No enclosing loop scope found");
        dropScopeVariables(*scope);
        return scope;
    }

    Scope *dropFuncScope()
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isFunctionScope()) {
            dropScopeVariables(*scope);
            scope = scope->parent;
        }
        assert(scope && "No enclosing function scope found");
        dropScopeVariables(*scope);
        return scope;
    }
};

gil::Function *GILGen::generateFunction(
    gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
)
{
    if (decl->getBody() == nullptr) {
        // If the function has no body, we skip it
        return nullptr;
    }
    return GILGenStmt(module, decl, globalCtx).ctx.getCurrentFunction();
}

static gil::Function *generateGlobalInitializerFunction(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
{
    return GILGenStmt(module, decl, globalCtx).ctx.getCurrentFunction();
}

gil::Global *GILGen::getOrCreateGlobal(
    gil::Module *module, ast::VarLetDecl *decl, llvm::BumpPtrAllocator &arena
)
{
    for (auto &g : module->getGlobals()) {
        if (g.getDecl() == decl) {
            return &g;
        }
    }

    auto *global = new (arena)
        gil::Global(decl->getName(), decl->getType(), nullptr, decl);
    module->addGlobal(global);
    return global;
}

gil::Global *GILGen::generateGlobal(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
{
    auto *global = getOrCreateGlobal(module, decl, globalCtx.arena);
    if (decl->getValue() != nullptr) {
        global->setInitializer(
            generateGlobalInitializerFunction(module, decl, globalCtx)
        );
    }
    return global;
}

gil::Module *GILGen::generateModule(
    ast::ModuleDecl *moduleDecl, llvm::BumpPtrAllocator &arena
)
{
    auto gilModule = new (arena) gil::Module(moduleDecl);
    GlobalContext globalCtx(gilModule, arena);

    // Generate GIL for all functions in the module
    for (auto decl : moduleDecl->getDecls()) {
        if (auto fn = llvm::dyn_cast<ast::FunctionDecl>(decl)) {
            if (fn->getBody() == nullptr) {
                // If the function has no body, we skip it
                continue;
            }
            generateFunction(gilModule, fn, globalCtx);
        } else if (auto varDecl = llvm::dyn_cast<ast::VarLetDecl>(decl)) {
            // Global variable or constant
            generateGlobal(gilModule, varDecl, globalCtx);
        }
    }

    // Generate GIL for all inlinable functions from other modules
    while (!globalCtx._inlinableFunctions.empty()) {
        auto fn = *globalCtx._inlinableFunctions.begin();
        globalCtx._inlinableFunctions.erase(fn);
        if (auto *gilFn = gilModule->getFunctionByDecl(fn);
            gilFn && gilFn->getBasicBlockCount()) {
            // Already generated
            continue;
        }
        generateFunction(gilModule, fn, globalCtx);
    }

    return gilModule;
}

gil::Function *GILGen::_generateFunctionTest(
    ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena
)
{
    auto gilModule = new (arena) gil::Module("test_module");
    GlobalContext globalCtx(gilModule, arena);
    return generateFunction(gilModule, decl, globalCtx);
}

} // namespace glu::gilgen
