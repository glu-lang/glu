#include "AST/ASTContext.hpp"
#include "AST/Stmts.hpp"
#include "AST/Types.hpp"
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

TEST(ASTContext_TypesMemoryArena, InternIntTy)
{
    ASTContext ctx;

    auto int32Signed
        = ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);
    auto int32Signed2
        = ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);

    auto int32Unsigned
        = ctx.getTypesMemoryArena().create<IntTy>(IntTy::Unsigned, 32);
    auto int64Signed
        = ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 64);

    ASSERT_EQ(int32Signed, int32Signed2);
    ASSERT_NE(int32Signed, int32Unsigned);
    ASSERT_NE(int32Signed, int64Signed);
}

TEST(ASTContext_TypesMemoryArena, InternFloatTy)
{
    ASTContext ctx;

    auto float32 = ctx.getTypesMemoryArena().create<FloatTy>(FloatTy::FLOAT);
    auto float32_2 = ctx.getTypesMemoryArena().create<FloatTy>(FloatTy::FLOAT);

    auto doubleTy = ctx.getTypesMemoryArena().create<FloatTy>(FloatTy::DOUBLE);

    auto halfTy = ctx.getTypesMemoryArena().create<FloatTy>(FloatTy::HALF);

    ASSERT_EQ(float32, float32_2);
    ASSERT_NE(float32, doubleTy);
    ASSERT_NE(float32, halfTy);
}
