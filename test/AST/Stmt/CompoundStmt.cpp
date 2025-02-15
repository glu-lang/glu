#include "Stmt/CompoundStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(CompoundStmt, CompoundStmtConstructor)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    CompoundStmt stmt(loc, std::move(stmts));

    ASSERT_TRUE(llvm::isa<CompoundStmt>(&stmt));
    ASSERT_TRUE(stmt.getStmts().empty());
}

TEST(CompoundStmt, CompoundStmtAddAndRemoveStmts)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    CompoundStmt stmt(loc, std::move(stmts));

    auto stmt1 = new CompoundStmt(loc, {});
    auto stmt2 = new CompoundStmt(loc, {});

    stmt.addStmt(stmt1);
    stmt.addStmt(stmt2);

    ASSERT_EQ(stmt.getStmts().size(), 2);
    ASSERT_EQ(stmt.getStmts()[0], stmt1);
    ASSERT_EQ(stmt.getStmts()[1], stmt2);
    ASSERT_EQ(stmt1->getParent(), &stmt);

    stmt.removeStmt(stmt1);

    ASSERT_EQ(stmt.getStmts().size(), 1);
    ASSERT_EQ(stmt.getStmts()[0], stmt2);

    delete stmt1;
    delete stmt2;
}
