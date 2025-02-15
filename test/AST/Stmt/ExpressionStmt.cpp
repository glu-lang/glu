#include "Stmt/ExpressionStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

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
