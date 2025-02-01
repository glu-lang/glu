#include "Stmt/WhileStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(WhileStmt, WhileStmtConstructor)
{
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);
    auto condition = nullptr;
    auto parent = nullptr;
    CompoundStmt body(loc, nullptr, std::move(stmts));

    WhileStmt stmt(loc, parent, condition, &body);

    ASSERT_TRUE(llvm::isa<WhileStmt>(&stmt));
    ASSERT_EQ(stmt.getCondition(), nullptr);
    ASSERT_EQ(stmt.getBody(), &body);
}