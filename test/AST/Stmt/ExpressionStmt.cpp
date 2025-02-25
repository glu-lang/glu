#include "Stmt/ExpressionStmt.hpp"
#include "ASTContext.hpp"
#include "Exprs.hpp"
#include "Types.hpp"

#include <llvm/ADT/APInt.h>
#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

class TestExpr : public ExprBase {
public:
    TestExpr() : ExprBase(NodeKind::LiteralExprKind, glu::SourceLocation(1)) { }
};

TEST(ExpressionStmt, ExpressionStmtConstructor)
{
    auto loc = glu::SourceLocation(42);
    auto expr = new TestExpr();

    ExpressionStmt stmt(loc, expr);

    ASSERT_TRUE(llvm::isa<ExpressionStmt>(&stmt));

    delete expr;
}

TEST(ExpressionStmt, CallExprStmt)
{
    ASTContext ctx;
    glu::SourceLocation loc(11);

    auto intty = ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 64);
    auto callee = ctx.getASTMemoryArena().create<TestExpr>();
    auto arg1 = ctx.getASTMemoryArena().create<LiteralExpr>(
        llvm::APInt(64, 1), intty, loc
    );
    auto arg2 = ctx.getASTMemoryArena().create<LiteralExpr>(
        llvm::APInt(64, 2), intty, loc
    );
    auto call = ctx.getASTMemoryArena().create<CallExpr>(
        loc, callee, llvm::SmallVector<ExprBase *> { arg1, arg2 }
    );

    ExpressionStmt stmt(loc, call);

    ASSERT_TRUE(llvm::isa<ExpressionStmt>(&stmt));
    ASSERT_EQ(stmt.getExpr(), call);
}
