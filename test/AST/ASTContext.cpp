#include "AST/ASTContext.hpp"
#include "AST/Stmts.hpp"
#include "Basic/SourceLocation.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

TEST(ASTContext, MemoryArena)
{
    ASTContext ctx;
    glu::SourceLocation loc(11);

    auto stmt = ctx.getASTMemoryArena().create<ReturnStmt>(loc, nullptr);
    auto boolType = ctx.getTypesMemoryArena().create<BoolTy>();
    auto sameBoolType = ctx.getTypesMemoryArena().create<BoolTy>();

    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(llvm::isa<ReturnStmt>(stmt));
    ASSERT_TRUE(llvm::isa<BoolTy>(boolType));
    ASSERT_EQ(boolType, sameBoolType);
}
