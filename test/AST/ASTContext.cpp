#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Stmt/ReturnStmt.hpp"
#include "Basic/SourceLocation.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

TEST(ASTContextTest, ASTContextTest)
{
    ASTContext ctx;
    glu::SourceLocation loc(11);

    ReturnStmt *stmt = ctx.createNode<ReturnStmt>(loc, nullptr);
    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(llvm::isa<ReturnStmt>(stmt));

    ctx.reset();
}
