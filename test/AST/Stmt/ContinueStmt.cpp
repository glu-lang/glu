#include "Stmt/ContinueStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(ContinueStmt, ContinueStmtConstructor)
{
    auto loc = glu::SourceLocation(42);

    ContinueStmt stmt(loc, nullptr);

    ASSERT_TRUE(llvm::isa<ContinueStmt>(&stmt));
}
