#ifndef GLU_GILGEN_GILGENLVALUE_HPP
#define GLU_GILGEN_GILGENLVALUE_HPP

#include "ASTVisitor.hpp"
#include "Context.hpp"
#include "Exprs.hpp"
#include "GIL/Instructions.hpp"
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
            auto ptrType = ctx.translateType(
                varLetDecl->getModule()
                    ->getContext()
                    ->getTypesMemoryArena()
                    .create<types::PointerTy>(varLetDecl->getType())
            );
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

        // Get the field information
        auto const &field = structType->getField(*fieldIndex);

        // Translate the field type to GIL type
        gil::Type fieldGilType = ctx.translateType(field->getType());
        gil::Type structGilType = ctx.translateType(structType);

        // Create the Member object
        gil::Member member(expr->getMemberName(), fieldGilType, structGilType);

        // Create and emit the StructFieldPtrInst using the context's build
        // method
        auto *structFieldPtrInst = ctx.buildStructFieldPtr(structPtr, member);

        return structFieldPtrInst->getResult(0);
    }
};

}

#endif /* GLU_GILGEN_GILGENLVALUE_HPP */
