#include "Stmt/ReturnStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

class TestExprBase : public ExprBase {
public:
    TestExprBase()
        : ExprBase(NodeKind::LiteralExprKind, glu::SourceLocation(1), nullptr)
    {
    }
};

TEST(ReturnStmt, ReturnStmtConstructor)
{
    auto loc = glu::SourceLocation(42);

    ReturnStmt stmt(loc, nullptr);

    ASSERT_TRUE(llvm::isa<ReturnStmt>(&stmt));
}

TEST(ReturnStmt, ReturnStmtConstructorWithReturnExpr)
{
    auto loc = glu::SourceLocation(42);
    ExprBase *returnExpr = new TestExprBase();

    ReturnStmt stmt(loc, nullptr, returnExpr);

    ASSERT_EQ(stmt.getReturnExpr(), returnExpr);

    delete returnExpr;
}
