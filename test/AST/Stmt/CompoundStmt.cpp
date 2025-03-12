#include "Stmt/CompoundStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(CompoundStmt, CompoundStmtConstructor)
{
    llvm::BumpPtrAllocator alloc;
    llvm::SmallVector<StmtBase *> stmts;
    auto loc = glu::SourceLocation(42);

    auto stmt = CompoundStmt::create(alloc, loc, std::move(stmts));

    ASSERT_TRUE(llvm::isa<CompoundStmt>(stmt));
    ASSERT_TRUE(stmt->getStmts().empty());
    ASSERT_EQ(stmt->getLocation(), loc);
}
