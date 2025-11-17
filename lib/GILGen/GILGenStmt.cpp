#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "GILGen.hpp"
#include "GILGenExprs.hpp"
#include "Scope.hpp"

#include <initializer_list>
#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenStmt : public ASTVisitor<GILGenStmt, void> {
    Context ctx;
    llvm::SmallVector<std::unique_ptr<Scope>, 8> scopes;

    /// Generates GIL code for the given function.
    GILGenStmt(
        gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx
    )
        : ctx(module, decl, globalCtx)
    {
        ctx.setSourceLocNode(decl);
        scopes.push_back(std::make_unique<Scope>(decl));
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
            ctx.buildStore(gilArg, alloca->getResult(0))
                ->setOwnershipKind(gil::StoreOwnershipKind::Init);
            ctx.buildDebug(
                   paramDecl->getName(), alloca->getResult(0),
                   gil::DebugBindingType::Arg
            )
                ->setLocation(paramDecl->getLocation());
            scope.variables.insert({ paramDecl, alloca->getResult(0) });
        }
        visitCompoundStmtNoScope(ctx.getASTFunction()->getBody());
        // at the end of the function, return void if appropriate
        dropFuncScope();
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

    Scope &getCurrentScope() { return *scopes.back(); }

    void pushScope(ast::CompoundStmt *stmt)
    {
        scopes.push_back(std::make_unique<Scope>(stmt, &getCurrentScope()));
    }

    void popScope()
    {
        auto &lastScope = getCurrentScope();
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
        pushScope(stmt);
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

        ctx.buildStore(rhs, lhs); // unknown ownership kind for now
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

        pushScope(stmt->getBody());
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
        auto *rangeExpr = stmt->getRange();
        auto rangeValue = expr(rangeExpr);

        auto rangeType = rangeValue.getType();
        auto rangeCopy = ctx.buildAlloca(rangeType)->getResult(0);
        ctx.buildStore(rangeValue, rangeCopy)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        auto beginValue = emitRefCall(
            stmt->getBeginFunc(),
            { ctx.buildLoadCopy(rangeType, rangeCopy)->getResult(0) }
        );
        auto iterType = beginValue.getType();
        auto iterVar = ctx.buildAlloca(iterType)->getResult(0);
        ctx.buildStore(beginValue, iterVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        auto endValue = emitRefCall(
            stmt->getEndFunc(),
            { ctx.buildLoadCopy(rangeType, rangeCopy)->getResult(0) }
        );
        auto endVar = ctx.buildAlloca(iterType)->getResult(0);
        ctx.buildStore(endValue, endVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        // This is the container scope for the range variables
        // It is not the loop body scope, as we don't want to drop the
        // range/iterator variables each iteration
        pushScope(stmt->getBody());
        getCurrentScope().unnamedAllocations.push_back(rangeCopy);
        getCurrentScope().unnamedAllocations.push_back(iterVar);
        getCurrentScope().unnamedAllocations.push_back(endVar);

        auto *condBB = ctx.buildBB("for.cond");
        auto *bodyBB = ctx.buildBB("for.body");
        auto *stepBB = ctx.buildBB("for.step");
        auto *endBB = ctx.buildBB("for.end");

        ctx.buildBr(condBB);

        // -- Condition --
        ctx.positionAtEnd(condBB);
        auto equalsValue = emitRefCall(
            stmt->getEqualityFunc(),
            { ctx.buildLoadCopy(iterType, iterVar)->getResult(0),
              ctx.buildLoadCopy(iterType, endVar)->getResult(0) }
        );
        ctx.buildCondBr(equalsValue, endBB, bodyBB);

        // -- Body --
        ctx.positionAtEnd(bodyBB);

        // This is the loop body scope
        pushScope(stmt->getBody());
        getCurrentScope().continueDestination = stepBB;
        getCurrentScope().breakDestination = endBB;

        auto *binding = stmt->getBinding();
        auto bindingType = ctx.translateType(binding->getType());
        auto bindingVar = ctx.buildAlloca(bindingType)->getResult(0);
        ctx.buildDebug(
            binding->getName(), bindingVar, gil::DebugBindingType::Let
        );
        auto bindingValue = emitRefCall(
            stmt->getDerefFunc(),
            { ctx.buildLoadCopy(iterType, iterVar)->getResult(0) }
        );
        ctx.buildStore(bindingValue, bindingVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);
        getCurrentScope().variables.insert({ binding, bindingVar });

        visitCompoundStmtNoScope(stmt->getBody());

        popScope();

        ctx.buildBr(stepBB);

        // -- Step --
        ctx.positionAtEnd(stepBB);
        auto nextValue = emitRefCall(
            stmt->getNextFunc(),
            { ctx.buildLoadCopy(iterType, iterVar)->getResult(0) }
        );
        ctx.buildStore(nextValue, iterVar);
        ctx.buildBr(condBB);

        // -- End --
        ctx.positionAtEnd(endBB);
        popScope(); // Drops range variables
    }

    void visitDeclStmt(DeclStmt *stmt)
    {
        auto *varDecl = llvm::cast<ast::VarLetDecl>(stmt->getDecl());

        auto ptr = ctx.buildAlloca(ctx.translateType(varDecl->getType()));
        ctx.buildDebug(
            varDecl->getName(), ptr->getResult(0),
            llvm::isa<ast::VarDecl>(varDecl) ? gil::DebugBindingType::Var
                                             : gil::DebugBindingType::Let
        );
        if (auto *value = varDecl->getValue()) {
            auto valueGIL = expr(value);
            ctx.buildStore(valueGIL, ptr->getResult(0))
                ->setOwnershipKind(gil::StoreOwnershipKind::Init);
        }
        getCurrentScope().variables.insert({ varDecl, ptr->getResult(0) });
    }

private:
    gil::Value emitRefCall(ast::RefExpr *ref, llvm::ArrayRef<gil::Value> args)
    {
        assert(ref && "Trying to call null RefExpr");
        gil::CallInst *callInst;
        if (auto *fn
            = llvm::dyn_cast<ast::FunctionDecl *>(ref->getVariable())) {
            callInst = ctx.buildCall(fn, args);
        } else {
            callInst = ctx.buildCall(expr(ref), args);
        }
        return callInst->getResult(0);
    }

    void dropScopeVariables(Scope &scope)
    {
        // Compiler generated drops have no debug location
        // Maybe they should have the location of the closing brace of the scope
        for (auto &[var, val] : scope.variables) {
            if (llvm::isa<ast::ParamDecl>(var)
                && ctx.getASTFunction()->getName() == "drop")
                continue; // Don't drop parameters in drop functions, otherwise
                          // infinite recursion
            ctx.buildDropPtr(ctx.translateType(var->getType()), val);
        }
        for (auto &val : scope.unnamedAllocations) {
            ctx.buildDropPtr(val.getType(), val);
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

gil::Function *generateFunction(
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

gil::Global *getOrCreateGlobal(gil::Module *module, ast::VarLetDecl *decl)
{
    for (auto &g : module->getGlobals()) {
        if (g.getDecl() == decl) {
            return &g;
        }
    }

    auto *global = new gil::Global(
        decl->getName(), decl->getType(), decl->getValue() != nullptr, decl
    );
    module->addGlobal(global);
    return global;
}

gil::Global *generateGlobal(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
{
    auto *global = getOrCreateGlobal(module, decl);
    if (decl->getValue() != nullptr) {
        global->setInitializer(
            generateGlobalInitializerFunction(module, decl, globalCtx)
        );
    }
    return global;
}

std::unique_ptr<gil::Module> generateModule(ast::ModuleDecl *moduleDecl)
{
    auto gilModule = std::make_unique<gil::Module>(moduleDecl);
    GlobalContext globalCtx(gilModule.get());

    // Generate GIL for all functions in the module
    for (auto decl : moduleDecl->getDecls()) {
        if (auto fn = llvm::dyn_cast<ast::FunctionDecl>(decl)) {
            if (fn->getBody() == nullptr) {
                // If the function has no body, we skip it
                continue;
            }
            generateFunction(gilModule.get(), fn, globalCtx);
        } else if (auto varDecl = llvm::dyn_cast<ast::VarLetDecl>(decl)) {
            // Global variable or constant
            generateGlobal(gilModule.get(), varDecl, globalCtx);
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
        generateFunction(gilModule.get(), fn, globalCtx);
    }

    return gilModule;
}

} // namespace glu::gilgen
