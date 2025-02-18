#include "AST/ASTContext.hpp"
#include "AST/Stmts.hpp"
#include "Basic/SourceLocation.hpp"

#include <gtest/gtest.h>
#include <llvm/Support/Casting.h>

using namespace glu::ast;
using namespace glu::types;

TEST(ASTContext_ASTMemoryArena, CreateReturnStmt)
{
    ASTContext ctx;
    glu::SourceLocation loc(11);

    auto stmt = ctx.getASTMemoryArena().create<ReturnStmt>(loc, nullptr);

    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(llvm::isa<ReturnStmt>(stmt));
}

TEST(ASTContext_TypesMemoryArena, InternBoolTy)
{
    ASTContext ctx;

    auto boolType = ctx.getTypesMemoryArena().create<BoolTy>();
    auto sameBoolType = ctx.getTypesMemoryArena().create<BoolTy>();

    ASSERT_TRUE(llvm::isa<BoolTy>(boolType));
    ASSERT_EQ(boolType, sameBoolType);
}

TEST(ASTContext_TypesMemoryArena, InternFunctionTy)
{
    ASTContext ctx;

    auto boolType = ctx.getTypesMemoryArena().create<BoolTy>();
    auto fctType = ctx.getTypesMemoryArena().create<FunctionTy>(
        std::vector<TypeBase *>(), boolType
    );
    auto sameFctType = ctx.getTypesMemoryArena().create<FunctionTy>(
        std::vector<TypeBase *>(), boolType
    );
    auto otherFctType = ctx.getTypesMemoryArena().create<FunctionTy>(
        std::vector<TypeBase *> { boolType }, fctType
    );

    ASSERT_EQ(fctType, sameFctType);
    ASSERT_NE(fctType, otherFctType);
}
