#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "GILGen.hpp"
#include "GILGenExprs.hpp"
#include "Scope.hpp"
#include "Sema/ScopeTable.hpp"

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
            auto *alloca = ctx.buildAlloca(paramDecl->getType());
            ctx.buildStore(gilArg, alloca->getResult(0))
                ->setOwnershipKind(gil::StoreOwnershipKind::Init);
            ctx.buildDebug(
                   paramDecl->getName(), alloca->getResult(0),
                   gil::DebugBindingType::Arg
            )
                ->setLocation(paramDecl->getLocation());
            scope.insertVariable(paramDecl, alloca->getResult(0));
        }
        visitCompoundStmtNoScope(ctx.getASTFunction()->getBody());
        // at the end of the function, return void if appropriate
        dropFuncScope();
        if (llvm::isa<types::VoidTy>(decl->getType()->getReturnType())) {
            ctx.buildRetVoid();
        } else {
            // If this is reachable, UnreachableInstChecker will report a
            // missing return error
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

    Scope &pushScope(ast::CompoundStmt *stmt)
    {
        scopes.push_back(std::make_unique<Scope>(stmt, &getCurrentScope()));
        return getCurrentScope();
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
        ctx.buildBr(scope->getBreakDestination());
        ctx.positionAtEnd(ctx.buildUnreachableBB());
    }

    void visitContinueStmt([[maybe_unused]] ContinueStmt *stmt)
    {
        Scope *scope = dropLoopScopes();
        ctx.buildBr(scope->getContinueDestination());
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

        pushScope(stmt->getBody()).setLoopDestinations(endBB, condBB);

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

        ctx.buildDropPtr(value);
    }

    void visitForStmt(ForStmt *stmt)
    {
        if (stmt->isArrayIteration()) {
            visitForStmtArray(stmt);
        } else {
            visitForStmtIterator(stmt);
        }
    }

    /// @brief Generates GIL code for a for loop iterating over a static array.
    /// Uses pointer-based iteration with inline begin/end computation.
    // TODO: is working correctly, but needs cleanup
    void visitForStmtArray(ForStmt *stmt)
    {
        auto *rangeExpr = stmt->getRange();
        auto arrayPtr = lvalue(rangeExpr);

        auto rangeType
            = llvm::cast<types::StaticArrayTy>(&*rangeExpr->getType());
        auto elementType = rangeType->getDataType();
        auto arraySize = rangeType->getSize();

        auto &typesArena = ctx.getASTContext()->getTypesMemoryArena();
        auto *elementPtrType = typesArena.create<types::PointerTy>(elementType);

        // begin = bitcast array pointer to element pointer
        auto beginValue
            = ctx.buildBitcast(elementPtrType, arrayPtr)->getResult(0);

        // end = begin + arraySize
        auto sizeValue = ctx.buildIntegerLiteral(
                                typesArena.create<types::IntTy>(
                                    types::IntTy::Unsigned, 64
                                ),
                                llvm::APInt(64, arraySize)
        )
                             ->getResult(0);
        auto endValue = ctx.buildPtrOffset(beginValue, sizeValue)->getResult(0);

        auto iterVar = ctx.buildAlloca(elementPtrType)->getResult(0);
        ctx.buildStore(beginValue, iterVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        auto endVar = ctx.buildAlloca(elementPtrType)->getResult(0);
        ctx.buildStore(endValue, endVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        // Container scope for the iterator variables
        auto &containerScope = pushScope(stmt->getBody());
        containerScope.addUnnamedAllocation(iterVar);
        containerScope.addUnnamedAllocation(endVar);

        auto *condBB = ctx.buildBB("for.cond");
        auto *bodyBB = ctx.buildBB("for.body");
        auto *stepBB = ctx.buildBB("for.step");
        auto *endBB = ctx.buildBB("for.end");

        ctx.buildBr(condBB);

        // -- Condition: iter == end --
        ctx.positionAtEnd(condBB);
        auto iterValue = ctx.buildLoadCopy(iterVar)->getResult(0);
        auto endCmp = ctx.buildLoadCopy(endVar)->getResult(0);

        // Compare pointers by casting to UInt64 and using builtin_eq
        auto *uintType
            = typesArena.create<types::IntTy>(types::IntTy::Unsigned, 64);
        auto iterAsInt
            = ctx.buildCastPtrToInt(uintType, iterValue)->getResult(0);
        auto endAsInt = ctx.buildCastPtrToInt(uintType, endCmp)->getResult(0);

        // Get builtin_eq for UInt64 directly from the builtins namespace
        auto *builtinEq = getBuiltinEqUInt64();
        auto *callInst = ctx.buildCall(builtinEq, { iterAsInt, endAsInt });
        auto equalsValue = callInst->getResult(0);
        ctx.buildCondBr(equalsValue, endBB, bodyBB);

        // -- Body --
        ctx.positionAtEnd(bodyBB);

        pushScope(stmt->getBody()).setLoopDestinations(endBB, stepBB);

        auto *binding = stmt->getBinding();
        auto bindingType = binding->getType();
        auto bindingVar = ctx.buildAlloca(bindingType)->getResult(0);
        ctx.buildDebug(
            binding->getName(), bindingVar, gil::DebugBindingType::Let
        );
        // Dereference the iterator pointer to get the current value
        auto currentIter = ctx.buildLoadCopy(iterVar)->getResult(0);
        auto bindingValue = ctx.buildLoadCopy(currentIter)->getResult(0);
        ctx.buildStore(bindingValue, bindingVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);
        getCurrentScope().insertVariable(binding, bindingVar);

        visitCompoundStmtNoScope(stmt->getBody());

        popScope();

        ctx.buildBr(stepBB);

        // -- Step: iter = iter + 1 --
        ctx.positionAtEnd(stepBB);
        auto iterForStep = ctx.buildLoadCopy(iterVar)->getResult(0);
        auto oneValue = ctx.buildIntegerLiteral(
                               typesArena.create<types::IntTy>(
                                   types::IntTy::Unsigned, 64
                               ),
                               llvm::APInt(64, 1)
        )
                            ->getResult(0);
        auto nextValue
            = ctx.buildPtrOffset(iterForStep, oneValue)->getResult(0);
        ctx.buildStore(nextValue, iterVar);
        ctx.buildBr(condBB);

        // -- End --
        ctx.positionAtEnd(endBB);
        popScope();
    }

    /// @brief Generates GIL code for a for loop using iterator functions.
    void visitForStmtIterator(ForStmt *stmt)
    {
        auto *rangeExpr = stmt->getRange();
        auto rangeValue = expr(rangeExpr);

        auto rangeType = rangeValue.getType();
        auto rangeCopy = ctx.buildAlloca(rangeType)->getResult(0);
        ctx.buildStore(rangeValue, rangeCopy)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        auto beginValue = emitRefCall(
            stmt->getBeginFunc(), { ctx.buildLoadCopy(rangeCopy)->getResult(0) }
        );
        auto iterType = beginValue.getType();
        auto iterVar = ctx.buildAlloca(iterType)->getResult(0);
        ctx.buildStore(beginValue, iterVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        auto endValue = emitRefCall(
            stmt->getEndFunc(), { ctx.buildLoadCopy(rangeCopy)->getResult(0) }
        );
        auto endVar = ctx.buildAlloca(iterType)->getResult(0);
        ctx.buildStore(endValue, endVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);

        // This is the container scope for the range variables
        // It is not the loop body scope, as we don't want to drop the
        // range/iterator variables each iteration
        auto &containerScope = pushScope(stmt->getBody());
        containerScope.addUnnamedAllocation(rangeCopy);
        containerScope.addUnnamedAllocation(iterVar);
        containerScope.addUnnamedAllocation(endVar);

        auto *condBB = ctx.buildBB("for.cond");
        auto *bodyBB = ctx.buildBB("for.body");
        auto *stepBB = ctx.buildBB("for.step");
        auto *endBB = ctx.buildBB("for.end");

        ctx.buildBr(condBB);

        // -- Condition --
        ctx.positionAtEnd(condBB);
        auto equalsValue = emitRefCall(
            stmt->getEqualityFunc(),
            { ctx.buildLoadCopy(iterVar)->getResult(0),
              ctx.buildLoadCopy(endVar)->getResult(0) }
        );
        ctx.buildCondBr(equalsValue, endBB, bodyBB);

        // -- Body --
        ctx.positionAtEnd(bodyBB);

        // This is the loop body scope
        pushScope(stmt->getBody()).setLoopDestinations(endBB, stepBB);

        auto *binding = stmt->getBinding();
        auto bindingType = binding->getType();
        auto bindingVar = ctx.buildAlloca(bindingType)->getResult(0);
        ctx.buildDebug(
            binding->getName(), bindingVar, gil::DebugBindingType::Let
        );
        auto bindingValue = emitRefCall(
            stmt->getDerefFunc(), { ctx.buildLoadCopy(iterVar)->getResult(0) }
        );
        ctx.buildStore(bindingValue, bindingVar)
            ->setOwnershipKind(gil::StoreOwnershipKind::Init);
        getCurrentScope().insertVariable(binding, bindingVar);

        visitCompoundStmtNoScope(stmt->getBody());

        popScope(); // Drops loop body variables

        ctx.buildBr(stepBB);

        // -- Step --
        ctx.positionAtEnd(stepBB);
        auto nextValue = emitRefCall(
            stmt->getNextFunc(), { ctx.buildLoadCopy(iterVar)->getResult(0) }
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

        auto ptr = ctx.buildAlloca(varDecl->getType());
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
        getCurrentScope().insertVariable(varDecl, ptr->getResult(0));
    }

private:
    /// @brief Gets the builtin_eq function for UInt64 from the builtins
    /// namespace. This is used for pointer comparison in array iteration.
    ast::FunctionDecl *getBuiltinEqUInt64()
    {
        auto *item = sema::ScopeTable::BUILTINS_NS.lookupItem("builtin_eq");
        assert(item && "builtin_eq not found in builtins namespace");

        auto &typesArena = ctx.getASTContext()->getTypesMemoryArena();
        auto *uintType
            = typesArena.create<types::IntTy>(types::IntTy::Unsigned, 64);

        // Find the overload that takes (UInt64, UInt64)
        for (auto &decl : item->decls) {
            if (auto *fn = llvm::dyn_cast<ast::FunctionDecl>(decl.item)) {
                auto *fnType = fn->getType();
                if (fnType->getParameterCount() == 2
                    && fnType->getParameter(0) == uintType
                    && fnType->getParameter(1) == uintType) {
                    return fn;
                }
            }
        }
        llvm_unreachable("builtin_eq(UInt64, UInt64) not found");
    }

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
        for (auto &val : llvm::reverse(scope.getAllocations())) {
            ctx.buildDropPtr(val);
        }
    }

    /// Find the enclosing loop scope, dropping variables from intermediate
    /// scopes
    Scope *dropLoopScopes()
    {
        Scope *scope = &getCurrentScope();
        while (scope && !scope->isLoopScope()) {
            dropScopeVariables(*scope);
            scope = scope->getParent();
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
            scope = scope->getParent();
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

gil::Function *generateGlobalInitializerFunction(
    gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx
)
{
    return GILGenStmt(module, decl, globalCtx).ctx.getCurrentFunction();
}

gil::Function *generateGlobalDestructorFunction(
    gil::Module *module, ast::VarLetDecl *decl, gil::Global *global,
    [[maybe_unused]] GlobalContext &globalCtx
)
{
    auto &typesArena = decl->getModule()->getContext()->getTypesMemoryArena();
    auto funcName = std::string(decl->getName()) + ".dtor";
    auto *funcType = typesArena.create<types::FunctionTy>(
        llvm::ArrayRef<glu::types::TypeBase *> {},
        typesArena.create<types::VoidTy>()
    );
    auto *function = new gil::Function(funcName, funcType, nullptr);
    module->addFunction(function);

    auto *bb = gil::BasicBlock::create("entry", llvm::ArrayRef<gil::Type> {});
    function->addBasicBlockAtEnd(bb);

    Context ctx(module, function);
    ctx.positionAtEnd(bb);
    auto *ptrType = typesArena.create<types::PointerTy>(decl->getType());
    auto *globalPtr = ctx.buildGlobalPtr(ptrType, global);
    ctx.buildDropPtr(globalPtr->getResult(0));
    ctx.buildRetVoid();

    return function;
}

} // namespace glu::gilgen
