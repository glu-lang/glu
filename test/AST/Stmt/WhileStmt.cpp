#include "Stmt/WhileStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

class TestExprBase : public ExprBase {
public:
    TestExprBase() : ExprBase(NodeKind::LiteralExprKind, glu::SourceLocation(1))
    {
    }
};

TEST(WhileStmt, WhileStmtConstructor)
{
    auto loc = glu::SourceLocation(42);
    auto condition = TestExprBase();
    CompoundStmt body(loc, {});

    WhileStmt stmt(loc, &condition, &body);

    ASSERT_TRUE(llvm::isa<WhileStmt>(&stmt));
    ASSERT_EQ(stmt.getCondition(), &condition);
    ASSERT_EQ(stmt.getBody(), &body);
}
