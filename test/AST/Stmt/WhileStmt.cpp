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
    llvm::BumpPtrAllocator alloc;
    auto loc = glu::SourceLocation(42);
    auto condition = TestExprBase();
    auto body = CompoundStmt::create(alloc, loc, {});

    WhileStmt stmt(loc, &condition, body);

    ASSERT_TRUE(llvm::isa<WhileStmt>(&stmt));
    ASSERT_EQ(stmt.getCondition(), &condition);
    ASSERT_EQ(stmt.getBody(), body);
}
