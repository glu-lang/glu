#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"
#include "AST/Types.hpp"
#include "Basic/SourceLocation.hpp"
#include "Basic/Tokens.hpp"

#include <llvm/ADT/APInt.h>
#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

class ASTChildReplacerVisitorTest : public ::testing::Test {
protected:
    ASTChildReplacerVisitorTest() : loc(10) { }

    void SetUp() override
    {
        context = std::make_unique<ASTContext>();

        // Create some basic types
        intType
            = context->getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);
        floatType = context->getTypesMemoryArena().create<FloatTy>(32);
        boolType = context->getTypesMemoryArena().create<BoolTy>();

        // Create some sample expressions
        intLiteral = context->getASTMemoryArena().create<LiteralExpr>(
            llvm::APInt(32, 42), intType, loc
        );

        floatLiteral = context->getASTMemoryArena().create<LiteralExpr>(
            llvm::APFloat(32.5f), floatType, loc
        );

        boolLiteral = context->getASTMemoryArena().create<LiteralExpr>(
            true, boolType, loc
        );

        // Create RefExpr for operators
        NamespaceIdentifier plusId { {}, "+" };
        NamespaceIdentifier minusId { {}, "-" };
        NamespaceIdentifier notId { {}, "!" };

        plusOp = context->getASTMemoryArena().create<RefExpr>(
            loc, plusId, nullptr
        );
        minusOp = context->getASTMemoryArena().create<RefExpr>(
            loc, minusId, nullptr
        );
        notOp
            = context->getASTMemoryArena().create<RefExpr>(loc, notId, nullptr);

        // Create a new expression for replacement tests
        newIntLiteral = context->getASTMemoryArena().create<LiteralExpr>(
            llvm::APInt(32, 100), intType, loc
        );
    }

    std::unique_ptr<ASTContext> context;
    glu::SourceLocation loc;

    // Types
    TypeBase *intType;
    TypeBase *floatType;
    TypeBase *boolType;

    // Literals
    LiteralExpr *intLiteral;
    LiteralExpr *floatLiteral;
    LiteralExpr *boolLiteral;
    LiteralExpr *newIntLiteral;

    // Operators
    RefExpr *plusOp;
    RefExpr *minusOp;
    RefExpr *notOp;
};

// Simple test to verify the test framework is working
TEST(ASTChildReplacerVisitorBasicTest, SimpleTest)
{
    EXPECT_EQ(1, 1);
}

// Test replaceChild method
TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInExpressionStmt)
{
    auto *exprStmt
        = context->getASTMemoryArena().create<ExpressionStmt>(loc, intLiteral);

    // Verify initial setup
    EXPECT_EQ(exprStmt->getExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), exprStmt);

    // Replace the child expression
    replaceChild(intLiteral, floatLiteral);

    // Verify the replacement
    EXPECT_EQ(exprStmt->getExpr(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), exprStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInReturnStmt)
{
    auto *returnStmt
        = context->getASTMemoryArena().create<ReturnStmt>(loc, intLiteral);

    // Verify initial setup
    EXPECT_EQ(returnStmt->getReturnExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), returnStmt);

    // Replace the child expression
    replaceChild(intLiteral, floatLiteral);

    // Verify the replacement
    EXPECT_EQ(returnStmt->getReturnExpr(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), returnStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInAssignStmt)
{
    glu::Token assignOp(glu::TokenKind::equalTok, "=");
    auto *assignStmt = context->getASTMemoryArena().create<AssignStmt>(
        loc, intLiteral, assignOp, floatLiteral
    );

    // Test replacing left operand
    EXPECT_EQ(assignStmt->getExprLeft(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), assignStmt);

    replaceChild(intLiteral, boolLiteral);

    EXPECT_EQ(assignStmt->getExprLeft(), boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), assignStmt);

    // Test replacing right operand
    EXPECT_EQ(assignStmt->getExprRight(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), assignStmt);

    replaceChild(floatLiteral, newIntLiteral);

    EXPECT_EQ(assignStmt->getExprRight(), newIntLiteral);
    EXPECT_EQ(newIntLiteral->getParent(), assignStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInIfStmt)
{
    auto *body = context->getASTMemoryArena().create<CompoundStmt>(
        loc, llvm::ArrayRef<StmtBase *> {}
    );
    auto *ifStmt = context->getASTMemoryArena().create<IfStmt>(
        loc, boolLiteral, body, nullptr
    );

    // Test replacing condition
    EXPECT_EQ(ifStmt->getCondition(), boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), ifStmt);

    replaceChild(boolLiteral, intLiteral);

    EXPECT_EQ(ifStmt->getCondition(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), ifStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInWhileStmt)
{
    auto *body = context->getASTMemoryArena().create<CompoundStmt>(
        loc, llvm::ArrayRef<StmtBase *> {}
    );
    auto *whileStmt = context->getASTMemoryArena().create<WhileStmt>(
        loc, boolLiteral, body
    );

    // Test replacing condition
    EXPECT_EQ(whileStmt->getCondition(), boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), whileStmt);

    replaceChild(boolLiteral, intLiteral);

    EXPECT_EQ(whileStmt->getCondition(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), whileStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInForStmt)
{
    auto *binding = context->getASTMemoryArena().create<ForBindingDecl>(
        loc, "i", intType
    );
    auto *body = context->getASTMemoryArena().create<CompoundStmt>(
        loc, llvm::ArrayRef<StmtBase *> {}
    );
    auto *forStmt = context->getASTMemoryArena().create<ForStmt>(
        loc, binding, intLiteral, body
    );

    // Test replacing range expression
    EXPECT_EQ(forStmt->getRange(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), forStmt);

    replaceChild(intLiteral, floatLiteral);

    EXPECT_EQ(forStmt->getRange(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), forStmt);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInCallExpr)
{
    NamespaceIdentifier funcId { {}, "func" };
    auto *callee
        = context->getASTMemoryArena().create<RefExpr>(loc, funcId, nullptr);

    llvm::SmallVector<ExprBase *, 2> args = { intLiteral, floatLiteral };
    auto *callExpr
        = context->getASTMemoryArena().create<CallExpr>(loc, callee, args);

    // Test replacing callee
    EXPECT_EQ(callExpr->getCallee(), callee);
    EXPECT_EQ(callee->getParent(), callExpr);

    replaceChild(callee, plusOp);

    EXPECT_EQ(callExpr->getCallee(), plusOp);
    EXPECT_EQ(plusOp->getParent(), callExpr);

    // Test replacing first argument
    EXPECT_EQ(callExpr->getArgs()[0], intLiteral);
    EXPECT_EQ(intLiteral->getParent(), callExpr);

    replaceChild(intLiteral, boolLiteral);

    EXPECT_EQ(callExpr->getArgs()[0], boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), callExpr);

    // Test replacing second argument
    EXPECT_EQ(callExpr->getArgs()[1], floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), callExpr);

    replaceChild(floatLiteral, newIntLiteral);

    EXPECT_EQ(callExpr->getArgs()[1], newIntLiteral);
    EXPECT_EQ(newIntLiteral->getParent(), callExpr);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInTernaryConditionalExpr)
{
    auto *ternaryExpr
        = context->getASTMemoryArena().create<TernaryConditionalExpr>(
            loc, boolLiteral, intLiteral, floatLiteral
        );

    // Test replacing condition
    EXPECT_EQ(ternaryExpr->getCondition(), boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), ternaryExpr);

    replaceChild(boolLiteral, newIntLiteral);

    EXPECT_EQ(ternaryExpr->getCondition(), newIntLiteral);
    EXPECT_EQ(newIntLiteral->getParent(), ternaryExpr);

    // Test replacing true expression
    EXPECT_EQ(ternaryExpr->getTrueExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), ternaryExpr);

    replaceChild(intLiteral, boolLiteral);

    EXPECT_EQ(ternaryExpr->getTrueExpr(), boolLiteral);
    EXPECT_EQ(boolLiteral->getParent(), ternaryExpr);

    // Test replacing false expression
    EXPECT_EQ(ternaryExpr->getFalseExpr(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), ternaryExpr);

    replaceChild(floatLiteral, intLiteral);

    EXPECT_EQ(ternaryExpr->getFalseExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), ternaryExpr);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInUnaryOpExpr)
{
    auto *unaryExpr = context->getASTMemoryArena().create<UnaryOpExpr>(
        loc, intLiteral, notOp
    );

    // Test replacing operand
    EXPECT_EQ(unaryExpr->getOperand(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), unaryExpr);

    replaceChild(intLiteral, floatLiteral);

    EXPECT_EQ(unaryExpr->getOperand(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), unaryExpr);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInCastExpr)
{
    auto *castExpr = context->getASTMemoryArena().create<CastExpr>(
        loc, intLiteral, floatType
    );

    // Test replacing casted expression
    EXPECT_EQ(castExpr->getCastedExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), castExpr);

    replaceChild(intLiteral, floatLiteral);

    EXPECT_EQ(castExpr->getCastedExpr(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), castExpr);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprInStructMemberExpr)
{
    auto *structMemberExpr
        = context->getASTMemoryArena().create<StructMemberExpr>(
            loc, intLiteral, "field"
        );

    // Test replacing struct expression
    EXPECT_EQ(structMemberExpr->getStructExpr(), intLiteral);
    EXPECT_EQ(intLiteral->getParent(), structMemberExpr);

    replaceChild(intLiteral, floatLiteral);

    EXPECT_EQ(structMemberExpr->getStructExpr(), floatLiteral);
    EXPECT_EQ(floatLiteral->getParent(), structMemberExpr);
}

TEST_F(ASTChildReplacerVisitorTest, ReplaceChildExprWithNullParent)
{
    // Test the case where the old expression has no parent
    auto *orphanExpr = context->getASTMemoryArena().create<LiteralExpr>(
        llvm::APInt(32, 999), intType, loc
    );

    // This should not crash or do anything
    replaceChild(orphanExpr, newIntLiteral);

    // The orphan expression should remain unchanged
    EXPECT_EQ(orphanExpr->getParent(), nullptr);
    EXPECT_EQ(newIntLiteral->getParent(), nullptr);
}

// Test individual replacement methods
