#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"
#include "LiteralVisitor.hpp"
#include "Scope.hpp"

#include "ASTVisitor.hpp"
#include "Exprs.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenExpr : public ASTVisitor<GILGenExpr, gil::Value> {
    Context &ctx;
    Scope &scope;

    GILGenExpr(Context &ctx, Scope &scope) : ctx(ctx), scope(scope) { }

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

    gil::Value visitCastExpr(CastExpr *expr)
    {
        using namespace glu::ast;

        // Generate code for the expression to be converted
        gil::Value sourceValue = visit(expr->getCastedExpr());

        // Get the destination type using our TypeTranslator visitor
        gil::Type destGilType = ctx.translateType(expr->getDestType());

        // Get source and destination types
        types::TypeBase *sourceType = expr->getCastedExpr()->getType();
        types::TypeBase *destType = expr->getDestType();

        // Cast between integer types
        if (llvm::isa<types::IntTy>(sourceType)
            && llvm::isa<types::IntTy>(destType)) {
            types::IntTy *sourceIntTy = llvm::cast<types::IntTy>(sourceType);
            types::IntTy *destIntTy = llvm::cast<types::IntTy>(destType);

            // Determine if this is an extension or truncation
            if (destIntTy->getBitWidth() < sourceIntTy->getBitWidth()) {
                // Integer truncation
                return ctx.buildIntTrunc(destGilType, sourceValue)
                    ->getResult(0);
            } else if (destIntTy->getBitWidth() > sourceIntTy->getBitWidth()) {
                // Integer extension (signed or unsigned)
                if (sourceIntTy->isSigned()) {
                    // Signed extension
                    return ctx.buildIntSext(destGilType, sourceValue)
                        ->getResult(0);
                } else {
                    // Unsigned extension
                    return ctx.buildIntZext(destGilType, sourceValue)
                        ->getResult(0);
                }
            } else {
                // Same width but different signedness - use bitcast
                if (sourceIntTy->getSignedness()
                    != destIntTy->getSignedness()) {
                    return ctx.buildBitcast(destGilType, sourceValue)
                        ->getResult(0);
                }
                // Same type, no conversion needed
                return sourceValue;
            }
        }

        // Cast between floating-point types
        if (llvm::isa<types::FloatTy>(sourceType)
            && llvm::isa<types::FloatTy>(destType)) {
            types::FloatTy *sourceFloatTy
                = llvm::cast<types::FloatTy>(sourceType);
            types::FloatTy *destFloatTy = llvm::cast<types::FloatTy>(destType);

            if (destFloatTy->getBitWidth() < sourceFloatTy->getBitWidth()) {
                // Float truncation
                return ctx.buildFloatTrunc(destGilType, sourceValue)
                    ->getResult(0);
            } else if (destFloatTy->getBitWidth()
                       > sourceFloatTy->getBitWidth()) {
                // Float extension
                return ctx.buildFloatExt(destGilType, sourceValue)
                    ->getResult(0);
            } else {
                // Same type, no conversion needed
                return sourceValue;
            }
        }

        // Cast between integers and pointers
        if (llvm::isa<types::IntTy>(sourceType)
            && llvm::isa<types::PointerTy>(destType)) {
            // Int to Ptr
            return ctx.buildCastIntToPtr(destGilType, sourceValue)
                ->getResult(0);
        }

        if (llvm::isa<types::PointerTy>(sourceType)
            && llvm::isa<types::IntTy>(destType)) {
            // Ptr to Int
            return ctx.buildCastPtrToInt(destGilType, sourceValue)
                ->getResult(0);
        }

        // Handle enum conversions
        if (llvm::isa<types::EnumTy>(sourceType)
            || llvm::isa<types::EnumTy>(destType)) {
            // If either type is an enum, use appropriate conversion
            if (llvm::isa<types::IntTy>(destType)) {
                // Enum to Int - use bitcast
                return ctx.buildBitcast(destGilType, sourceValue)->getResult(0);
            } else if (llvm::isa<types::IntTy>(sourceType)) {
                // Int to Enum - use bitcast
                return ctx.buildBitcast(destGilType, sourceValue)->getResult(0);
            }
        }

        // Cast between pointers of different types or other incompatible types
        // Use bitcast for these situations
        return ctx.buildBitcast(destGilType, sourceValue)->getResult(0);
    }

    gil::Value visitStructInitializerExpr(ast::StructInitializerExpr *expr)
    {
        auto initializerFields = expr->getFields();
        auto defaultFieldDecls
            = llvm::cast<types::StructTy>(expr->getType())->getFields();

        llvm::SmallVector<gil::Value, 4> fields;
        fields.reserve(defaultFieldDecls.size());

        // Fill fields with provided values or defaults
        for (size_t i = 0; i < defaultFieldDecls.size(); ++i) {
            if (i < initializerFields.size()) {
                fields.push_back(visit(initializerFields[i]));
            } else {
                auto *defaultValue = defaultFieldDecls[i]->getValue();
                assert(defaultValue && "Field has no default value");
                fields.push_back(visit(defaultValue));
            }
        }

        return ctx
            .buildStructCreate(ctx.translateType(expr->getType()), fields)
            ->getResult(0);
    }

    gil::Value visitStructMemberExpr(ast::StructMemberExpr *expr)
    {
        gil::Value structValue = visit(expr->getStructExpr());
        llvm::StringRef memberName = expr->getMemberName();
        auto *structType
            = llvm::cast<types::StructTy>(structValue.getType().getType());
        gil::Member member(
            memberName, ctx.translateType(expr->getType()),
            ctx.translateType(structType)
        );

        return ctx.buildStructExtract(structValue, member)->getResult(0);
    }

    gil::Value visitBinaryOpExpr(BinaryOpExpr *expr)
    {
        using namespace glu::ast;

        // Get the token kind for the operator
        llvm::StringRef op = expr->getOperator()->getIdentifier();

        bool isAndOp = op == "&&";
        bool isOrOp = op == "||";

        // Special case for short-circuit logical operators (&& and ||)
        if (isAndOp || isOrOp) {
            // Generate code for the left operand first
            gil::Value leftValue = visit(expr->getLeftOperand());

            // Create a basic block for the result (with one argument of the
            // value type)
            gil::BasicBlock *resultBB
                = ctx.buildBB("logical.result", { leftValue.getType() });

            // Create the basic block for evaluating the right operand
            gil::BasicBlock *evalRightBB
                = ctx.buildBB(isAndOp ? "and.right" : "or.right");

            // Set up the branches with appropriate conditions
            if (isAndOp) {
                // AND: If left is false, jump to result with "false", otherwise
                // evaluate right
                ctx.buildCondBr(
                    leftValue, evalRightBB, resultBB, {}, { leftValue }
                );
            } else { // OR
                // OR: If left is true, jump to result with "true", otherwise
                // evaluate right
                ctx.buildCondBr(
                    leftValue, resultBB, evalRightBB, { leftValue }, {}
                );
            }

            // Evaluate right operand in its own block
            ctx.positionAtEnd(evalRightBB);
            gil::Value rightValue = visit(expr->getRightOperand());
            ctx.buildBr(resultBB, { rightValue });

            // Position at the result block and return its argument
            ctx.positionAtEnd(resultBB);
            return resultBB->getArgument(0);
        }

        // Standard case for non-short-circuit operators
        // Generate code for the left and right operands
        gil::Value leftValue = visit(expr->getLeftOperand());
        gil::Value rightValue = visit(expr->getRightOperand());

        llvm::SmallVector<gil::Value, 2> args { leftValue, rightValue };
        return ctx
            .buildCall(
                expr->getOperator()->getVariable().get<FunctionDecl *>(), args
            )
            ->getResult(0);
    }

    gil::Value visitTernaryConditionalExpr(TernaryConditionalExpr *expr)
    {
        // Sema must set the expression type and insert any implicit
        // conversions.
        auto *resAstTy = expr->getType();
        assert(resAstTy && "Ternary type must be set by Sema");
        gil::Type resGilTy = ctx.translateType(resAstTy);

        // Evaluate condition
        gil::Value condValue = visit(expr->getCondition());

        // Create blocks
        auto *thenBB = ctx.buildBB("ternary.then");
        auto *elseBB = ctx.buildBB("ternary.else");
        auto *mergeBB = ctx.buildBB("ternary.result", { resGilTy });

        // Branch on condition
        ctx.buildCondBr(condValue, thenBB, elseBB);

        // Then branch
        ctx.positionAtEnd(thenBB);
        gil::Value trueValue = visit(expr->getTrueExpr());
        ctx.buildBr(mergeBB, { trueValue });

        // Else branch
        ctx.positionAtEnd(elseBB);
        gil::Value falseValue = visit(expr->getFalseExpr());
        ctx.buildBr(mergeBB, { falseValue });

        // Merge
        ctx.positionAtEnd(mergeBB);
        return mergeBB->getArgument(0);
    }

    gil::Value visitUnaryOpExpr(UnaryOpExpr *expr)
    {
        using namespace glu::ast;

        // Generate code for the operand
        gil::Value operandValue = visit(expr->getOperand());

        // For now, create a call to the appropriate operator function by name
        llvm::SmallVector<gil::Value, 1> args { operandValue };
        return ctx
            .buildCall(
                expr->getOperator()->getVariable().get<FunctionDecl *>(), args
            )
            ->getResult(0);
    }

    gil::Value visitCallExpr(CallExpr *expr)
    {
        using namespace glu::ast;

        // Generate code for the callee expression and its value
        ExprBase *calleeExpr = expr->getCallee();

        // Generate code for each argument
        llvm::SmallVector<gil::Value, 4> argValues;
        for (ExprBase *arg : expr->getArgs()) {
            argValues.push_back(visit(arg));
        }

        gil::CallInst *callInst = nullptr;

        if (auto *ref = llvm::dyn_cast_if_present<RefExpr>(calleeExpr)) {
            if (auto *directCallee
                = llvm::dyn_cast_if_present<FunctionDecl *>(ref->getVariable()
                )) {
                callInst = ctx.buildCall(directCallee, argValues);
            }
        }
        if (!callInst) {
            callInst = ctx.buildCall(visit(calleeExpr), argValues);
        }
        if (callInst->getResultCount() == 0) {
            // For void, return an empty value
            return gil::Value::getEmptyKey();
        }
        return callInst->getResult(0);
    }

    gil::Value visitLiteralExpr(LiteralExpr *expr)
    {
        // Get the literal value and type
        auto literalValue = expr->getValue();
        auto type = ctx.translateType(expr->getType());

        // Use the LiteralVisitor to generate the appropriate GIL instruction
        return LiteralVisitor(ctx, type).visit(literalValue);
    }

    gil::Value visitRefExpr(RefExpr *expr)
    {
        // Look up the variable in the current scope
        auto varDecl = expr->getVariable();
        if (auto fn = llvm::dyn_cast<FunctionDecl *>(varDecl)) {
            // FIXME: probably wrong between function  typeand function pointer
            // TODO: need gil::Function from ast::FunctionDecl
            // return ctx.buildFunctionPtr(
            //     ctx.translateType(fn->getType()), fn
            // )->getResult(0);
            llvm_unreachable("Function references not implemented yet");
        }
        auto varValue = scope.lookupVariable(llvm::cast<VarLetDecl *>(varDecl));

        assert(varValue && "Variable not found in current scope");
        return ctx.buildLoad(ctx.translateType(expr->getType()), *varValue)
            ->getResult(0);
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
