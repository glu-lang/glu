#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Stmts.hpp"
#include "AST/Types.hpp"
#include "Basic/SourceLocation.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

TEST(ASTContextTest, ASTContextTest)
{
    ASTContext ctx;
    glu::SourceLocation loc(11);

    auto stmt = ctx.getASTMemoryArena().createNode<ReturnStmt>(loc, nullptr);
    auto type = ctx.getTypesMemoryArena().createNode<BoolTy>();

    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(llvm::isa<ReturnStmt>(stmt));
    ASSERT_TRUE(llvm::isa<BoolTy>(type));
}
