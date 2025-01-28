#include "Stmt/BreakStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(BreakStmt, BreakStmtConstructor)
{
    auto loc = glu::SourceLocation(42);

    BreakStmt stmt(loc, nullptr);

    ASSERT_TRUE(llvm::isa<BreakStmt>(&stmt));
}
