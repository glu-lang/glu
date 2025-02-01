#include "Stmt/ReturnStmt.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(ReturnStmt, ReturnStmtConstructor)
{
    auto loc = glu::SourceLocation(42);

    ReturnStmt stmt(loc, nullptr);

    ASSERT_TRUE(llvm::isa<ReturnStmt>(&stmt));
}
