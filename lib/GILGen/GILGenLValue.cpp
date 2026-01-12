#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "Exprs.hpp"
#include "GIL/Instructions.hpp"
#include "GILGenExprs.hpp"
#include "Scope.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenLValue : public ASTVisitor<GILGenLValue, gil::Value> {
    Context &ctx;
    Scope &scope;

    GILGenLValue(Context &ctx, Scope &scope) : ctx(ctx), scope(scope) { }

    void beforeVisitNode(ASTNode *node) { ctx.setSourceLocNode(node); }

    void afterVisitNode(ASTNode *node)
    {
        ctx.setSourceLocNode(node->getParent());
    }

    gil::Value visitASTNode([[maybe_unused]] ASTNode *expr)
    {
        assert(false && "Unknown expression kind");
        return gil::Value::getEmptyKey();
    }

    gil::Value visitRefExpr(RefExpr *expr)
    {
        // Function references cannot be used as lvalues
        assert(
            llvm::isa<VarLetDecl *>(expr->getVariable())
            && "Function references cannot be used as lvalues"
        );

        auto varLetDecl = llvm::cast<VarLetDecl *>(expr->getVariable());

        if (varLetDecl->getParent() == varLetDecl->getModule()) {
            // Global variable - use global_ptr instruction
            gil::Global *globalVar = ctx.getOrCreateGlobal(varLetDecl);
            assert(globalVar && "Global variable not found in module scope");
            auto ptrType = varLetDecl->getModule()
                               ->getContext()
                               ->getTypesMemoryArena()
                               .create<types::PointerTy>(varLetDecl->getType());
            auto globalPtr = ctx.buildGlobalPtr(ptrType, globalVar);
            return globalPtr->getResult(0);
        }

        auto var = scope.lookupVariable(varLetDecl);
        assert(var && "Variable not found in current scope");
        return *var;
    }

    gil::Value visitStructMemberExpr(StructMemberExpr *expr)
    {
        // Get the struct lvalue (pointer to the struct)
        gil::Value structPtr = visit(expr->getStructExpr());

        // Get the struct type from the AST
        auto *structType
            = llvm::cast<types::StructTy>(expr->getStructExpr()->getType());

        // Find the field index by name
        auto fieldIndex = structType->getFieldIndex(expr->getMemberName());
        if (!fieldIndex.has_value()) {
            // This should have been caught during semantic analysis
            assert(false && "Struct field not found");
            return gil::Value::getEmptyKey();
        }

        // Use the resolved type from the expression, not the field declaration
        // This handles template parameter substitution (e.g., T -> Int)
        auto *memberType = expr->getType();

        // Create the Member object
        gil::Member member(expr->getMemberName(), memberType, structType);

        // Create and emit the StructFieldPtrInst using the context's build
        // method
        auto *structFieldPtrInst = ctx.buildStructFieldPtr(structPtr, member);

        return structFieldPtrInst->getResult(0);
    }

    gil::Value visitUnaryOpExpr(UnaryOpExpr *expr)
    {
        auto *ptrType
            = llvm::dyn_cast<types::PointerTy>(expr->getOperand()->getType());

        if (ptrType && expr->getOperator()->getIdentifier() == ".*") {
            return visitExpr(ctx, scope, expr->getOperand());
        }
        llvm_unreachable("Invalid lvalue expression");
    }

    gil::Value visitBinaryOpExpr(BinaryOpExpr *expr)
    {
        auto op = expr->getOperator()->getIdentifier();

        // Special case for pointer subscript operator
        if (op == "[" && expr->getOperator()->getVariable().isNull()) {
            gil::Value ptrValue = visitExpr(ctx, scope, expr->getLeftOperand());
            gil::Value offsetValue
                = visitExpr(ctx, scope, expr->getRightOperand());
            auto *ptrOffset = ctx.buildPtrOffset(ptrValue, offsetValue);
            return ptrOffset->getResult(0);
        }

        assert(false && "Invalid lvalue expression");
        return gil::Value::getEmptyKey();
    }
};

gil::Value visitLValue(Context &ctx, Scope &scope, ExprBase *expr)
{
    return GILGenLValue(ctx, scope).visit(expr);
}

} // namespace glu::gilgen
