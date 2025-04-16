#ifndef GLU_GILGEN_GILGENEXPR_HPP
#define GLU_GILGEN_GILGENEXPR_HPP

#include "Context.hpp"

#include "ASTVisitor.hpp"
#include "Exprs.hpp"

namespace glu::gilgen {

using namespace glu::ast;

struct GILGenExpr : public ASTVisitor<GILGenExpr, gil::Value> {
    Context &ctx;

    GILGenExpr(Context &ctx) : ctx(ctx) { }

    gil::Value visitExprBase([[maybe_unused]] ExprBase *expr)
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

    gil::Value visitBinaryOpExpr(BinaryOpExpr *expr)
    {
        using namespace glu::ast;

        // Get the token kind for the operator
        TokenKind opKind = expr->getOperator().getKind();

        // Special case for short-circuit logical operators (&& and ||)
        if (opKind == TokenKind::andOpTok || opKind == TokenKind::orOpTok) {
            // Generate code for the left operand first
            gil::Value leftValue = visit(expr->getLeftOperand());

            // Create a basic block for the result
            gil::BasicBlock *resultBB = ctx.buildBB("logical.result");

            // Create the basic block for evaluating the right operand
            gil::BasicBlock *evalRightBB = ctx.buildBB(
                opKind == TokenKind::andOpTok ? "and.right" : "or.right"
            );

            // Set up the branches with appropriate conditions
            if (opKind == TokenKind::andOpTok) {
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

        // Get the lexeme directly from the token (operator name without @
        // prefix)
        std::string opName = expr->getOperator().getLexeme().str();

        // TODO: When sema is implemented, this should be replaced with:
        // ast::FunctionDecl *opFunc = expr->getOperatorFunction();
        // return ctx.buildCall(opFunc, args);

        // For now, create a call to the appropriate operator function by name
        llvm::SmallVector<gil::Value, 2> args { leftValue, rightValue };
        return ctx.buildCall(opName, args)->getResult(0);
    }

    gil::Value visitUnaryOpExpr(UnaryOpExpr *expr)
    {
        using namespace glu::ast;

        // Generate code for the operand
        gil::Value operandValue = visit(expr->getOperand());

        // Get the lexeme directly from the token
        std::string opName = expr->getOperator().getLexeme().str();

        // TODO: When sema is implemented, this should be replaced with:
        // ast::FunctionDecl *opFunc = expr->getOperatorFunction();
        // return ctx.buildCall(opFunc, args);

        // For now, create a call to the appropriate operator function by name
        llvm::SmallVector<gil::Value, 1> args { operandValue };
        return ctx.buildCall(opName, args)->getResult(0);
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILGENEXPR_HPP
