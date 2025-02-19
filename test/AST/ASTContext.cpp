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

TEST(ASTContext_TypesMemoryArena, InternDynamicArrayTy)
{
    ASTContext ctx;
    auto elementType = ctx.getTypesMemoryArena().create<BoolTy>();

    auto dynArray1
        = ctx.getTypesMemoryArena().create<DynamicArrayTy>(elementType);
    auto dynArray2
        = ctx.getTypesMemoryArena().create<DynamicArrayTy>(elementType);
    auto dynArrayDiff = ctx.getTypesMemoryArena().create<DynamicArrayTy>(
        ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32)
    );

    ASSERT_EQ(dynArray1, dynArray2);
    ASSERT_NE(dynArray1, dynArrayDiff);
}

TEST(ASTContext_TypesMemoryArena, InternEnumTy)
{
    ASTContext ctx;
    llvm::SmallVector<EnumTy::Case> cases {
        EnumTy::Case { "Red", llvm::APInt(32, 1) },
        EnumTy::Case { "Blue", llvm::APInt(32, 2) }
    };

    auto enum1 = ctx.getTypesMemoryArena().create<EnumTy>(
        "Color", cases, glu::SourceLocation(100)
    );
    auto enum2 = ctx.getTypesMemoryArena().create<EnumTy>(
        "Color", cases, glu::SourceLocation(100)
    );
    auto enumDiff = ctx.getTypesMemoryArena().create<EnumTy>(
        "Color", cases, glu::SourceLocation(101)
    );

    ASSERT_EQ(enum1, enum2);
    ASSERT_NE(enum1, enumDiff);
}

TEST(ASTContext_TypesMemoryArena, InternPointerTy)
{
    ASTContext ctx;
    auto baseType = ctx.getTypesMemoryArena().create<BoolTy>();

    auto ptr1 = ctx.getTypesMemoryArena().create<PointerTy>(baseType);
    auto ptr2 = ctx.getTypesMemoryArena().create<PointerTy>(baseType);
    auto ptrDiff = ctx.getTypesMemoryArena().create<PointerTy>(
        ctx.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32)
    );

    ASSERT_EQ(ptr1, ptr2);
    ASSERT_NE(ptr1, ptrDiff);
}

TEST(ASTContext_TypesMemoryArena, InternStaticArrayTy)
{
    ASTContext ctx;
    auto elementType = ctx.getTypesMemoryArena().create<BoolTy>();

    auto arr1
        = ctx.getTypesMemoryArena().create<StaticArrayTy>(elementType, 10);
    auto arr2
        = ctx.getTypesMemoryArena().create<StaticArrayTy>(elementType, 10);
    auto arrDiff
        = ctx.getTypesMemoryArena().create<StaticArrayTy>(elementType, 20);

    ASSERT_EQ(arr1, arr2);
    ASSERT_NE(arr1, arrDiff);
}

TEST(ASTContext_TypesMemoryArena, InternStructTy)
{
    ASTContext ctx;
    llvm::SmallVector<StructTy::Field> fields
        = { { "a", ctx.getTypesMemoryArena().create<BoolTy>() } };

    auto struct1
        = ctx.getTypesMemoryArena().create<StructTy>("MyStruct", fields,
        200);
    auto struct2
        = ctx.getTypesMemoryArena().create<StructTy>("MyStruct", fields,
        200);
    auto structDiff
        = ctx.getTypesMemoryArena().create<StructTy>("MyStruct", fields,
        201);

    ASSERT_EQ(struct1, struct2);
    ASSERT_NE(struct1, structDiff);
}

TEST(ASTContext_TypesMemoryArena, InternTypeAliasTy)
{
    ASTContext ctx;
    auto wrappedType = ctx.getTypesMemoryArena().create<BoolTy>();

    auto alias1 = ctx.getTypesMemoryArena().create<TypeAliasTy>(
        wrappedType, "Alias", 300
    );
    auto alias2 = ctx.getTypesMemoryArena().create<TypeAliasTy>(
        wrappedType, "Alias", 300
    );
    auto aliasDiff = ctx.getTypesMemoryArena().create<TypeAliasTy>(
        wrappedType, "Alias", 301
    );

    ASSERT_EQ(alias1, alias2);
    ASSERT_NE(alias1, aliasDiff);
}

TEST(ASTContext_TypesMemoryArena, InternUnresolvedNameTy)
{
    ASTContext ctx;

    auto name1 = ctx.getTypesMemoryArena().create<UnresolvedNameTy>("Foo");
    auto name2 = ctx.getTypesMemoryArena().create<UnresolvedNameTy>("Foo");
    auto nameDiff = ctx.getTypesMemoryArena().create<UnresolvedNameTy>("Bar");

    ASSERT_EQ(name1, name2);
    ASSERT_NE(name1, nameDiff);
}
