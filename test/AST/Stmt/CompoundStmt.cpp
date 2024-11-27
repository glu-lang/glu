#include "Stmt/CompoundStmt.hpp"

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(CompoundStmt, CompoundStmtConstructor)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    CompoundStmt stmt(loc, nullptr, std::move(stmts));

    ASSERT_TRUE(stmt.getStmts().empty());
}

TEST(CompoundStmt, CompoundStmtAddAndRemoveStmts)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    CompoundStmt stmt(loc, nullptr, std::move(stmts));

    auto stmt1 = new CompoundStmt(loc, nullptr, {});
    auto stmt2 = new CompoundStmt(loc, nullptr, {});

    stmt.addStmt(stmt1);
    stmt.addStmt(stmt2);

    ASSERT_EQ(stmt.getStmts().size(), 2);
    ASSERT_EQ(stmt.getStmts()[0], stmt1);
    ASSERT_EQ(stmt.getStmts()[1], stmt2);
    // TODO: Check if parent is set correctly
    // ASSERT_EQ(stmt1->getParent(), &stmt);

    stmt.removeStmt(stmt1);

    ASSERT_EQ(stmt.getStmts().size(), 1);
    ASSERT_EQ(stmt.getStmts()[0], stmt2);

    delete stmt1;
    delete stmt2;
}

TEST(ConstCompoundStmt, ConstCompoundStmt)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    CompoundStmt const stmt(loc, nullptr, std::move(stmts));

    ASSERT_TRUE(stmt.getStmts().empty());
}
