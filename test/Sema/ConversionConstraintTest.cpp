#include "AST/ASTContext.hpp"
#include "AST/Decl/ModuleDecl.hpp"
#include "AST/Expr/LiteralExpr.hpp"
#include "AST/Types/BoolTy.hpp"
#include "AST/Types/EnumTy.hpp"
#include "AST/Types/FloatTy.hpp"
#include "AST/Types/IntTy.hpp"
#include "AST/Types/PointerTy.hpp"
#include "AST/Types/StaticArrayTy.hpp"
#include "AST/Types/TypeVariableTy.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"
#include "Sema/ScopeTable.hpp"
#include "gtest/gtest.h"

using namespace glu;
using namespace glu::sema;
using namespace glu::types;

class ConversionConstraintTest : public ::testing::Test {
protected:
    std::unique_ptr<ast::ASTContext> context;
    std::unique_ptr<SourceManager> sourceManager;
    std::unique_ptr<DiagnosticManager> diagManager;
    std::unique_ptr<ScopeTable> scopeTable;
    std::unique_ptr<ConstraintSystem> cs;
    ast::ModuleDecl *moduleDecl;
    llvm::BumpPtrAllocator allocator;

    // Common types for testing
    IntTy *int8Type;
    IntTy *int32Type;
    IntTy *int64Type;
    FloatTy *float32Type;
    FloatTy *float64Type;
    BoolTy *boolType;
    EnumTy *enumType;
    PointerTy *int32PtrType;
    PointerTy *int64PtrType;
    StaticArrayTy *int32ArrayType;
    TypeVariableTy *typeVar;

    void SetUp() override
    {
        sourceManager = std::make_unique<SourceManager>();
        diagManager = std::make_unique<DiagnosticManager>(*sourceManager);
        context = std::make_unique<ast::ASTContext>();

        // Create a simple module declaration for ScopeTable
        SourceLocation loc(0);
        llvm::ArrayRef<ast::DeclBase *> emptyDecls;
        moduleDecl = ast::ModuleDecl::create(
            allocator, loc, "test_module", emptyDecls, context.get()
        );

        scopeTable = std::make_unique<ScopeTable>(moduleDecl);
        cs = std::make_unique<ConstraintSystem>(
            scopeTable.get(), *diagManager, context.get()
        );

        // Create common types
        auto &arena = context->getTypesMemoryArena();
        int8Type = arena.create<IntTy>(IntTy::Signed, 8);
        int32Type = arena.create<IntTy>(IntTy::Signed, 32);
        int64Type = arena.create<IntTy>(IntTy::Signed, 64);
        float32Type = arena.create<FloatTy>(32);
        float64Type = arena.create<FloatTy>(64);
        boolType = arena.create<BoolTy>();
        // Create enum type using the static create method
        llvm::ArrayRef<Case> emptyCases;
        enumType = EnumTy::create(arena.getAllocator(), "TestEnum", emptyCases, SourceLocation(0));
        int32PtrType = arena.create<PointerTy>(int32Type);
        int64PtrType = arena.create<PointerTy>(int64Type);
        int32ArrayType = arena.create<StaticArrayTy>(int32Type, 10);
        typeVar = arena.create<TypeVariableTy>();

        cs->addTypeVariable(typeVar);
    }

    void TearDown() override
    {
        // Clean up in reverse order
        cs.reset();
        scopeTable.reset();
        diagManager.reset();
        sourceManager.reset();
        context.reset();
    }

    /// @brief Helper function to test implicit conversions
    bool testImplicitConversion(glu::types::Ty fromType, glu::types::Ty toType)
    {
        SystemState state;
        return cs->isValidConversion(fromType, toType, state, false);
    }

    /// @brief Helper function to test explicit conversions (checked casts)
    bool testExplicitConversion(glu::types::Ty fromType, glu::types::Ty toType)
    {
        SystemState state;
        return cs->isValidConversion(fromType, toType, state, true);
    }
};

// Test integer conversions
TEST_F(ConversionConstraintTest, IntegerWidening)
{
    // int8 -> int32 (widening) should be allowed implicitly
    EXPECT_TRUE(testImplicitConversion(int8Type, int32Type));
    
    // int32 -> int64 (widening) should be allowed implicitly
    EXPECT_TRUE(testImplicitConversion(int32Type, int64Type));
    
    // int8 -> int64 (widening) should be allowed implicitly
    EXPECT_TRUE(testImplicitConversion(int8Type, int64Type));
}

TEST_F(ConversionConstraintTest, IntegerNarrowing)
{
    // int64 -> int32 (narrowing) should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int64Type, int32Type));
    
    // int32 -> int8 (narrowing) should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int32Type, int8Type));
    
    // int64 -> int8 (narrowing) should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int64Type, int8Type));
}

TEST_F(ConversionConstraintTest, IntegerNarrowingExplicit)
{
    // int64 -> int32 (narrowing) should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int64Type, int32Type));
    
    // int32 -> int8 (narrowing) should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int32Type, int8Type));
    
    // int64 -> int8 (narrowing) should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int64Type, int8Type));
}

// Test float conversions
TEST_F(ConversionConstraintTest, FloatWidening)
{
    // float32 -> float64 (widening) should be allowed implicitly
    EXPECT_TRUE(testImplicitConversion(float32Type, float64Type));
}

TEST_F(ConversionConstraintTest, FloatNarrowing)
{
    // float64 -> float32 (narrowing) should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(float64Type, float32Type));
}

TEST_F(ConversionConstraintTest, FloatNarrowingExplicit)
{
    // float64 -> float32 (narrowing) should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(float64Type, float32Type));
}

// Test array to pointer conversions
TEST_F(ConversionConstraintTest, ArrayToPointer)
{
    // int32[10] -> int32* should be allowed implicitly
    EXPECT_TRUE(testImplicitConversion(int32ArrayType, int32PtrType));
    
    // int32[10] -> int64* should NOT be allowed (different element type)
    EXPECT_FALSE(testImplicitConversion(int32ArrayType, int64PtrType));
}

// Test pointer conversions
TEST_F(ConversionConstraintTest, PointerToPointer)
{
    // Same pointer type should be allowed
    EXPECT_TRUE(testImplicitConversion(int32PtrType, int32PtrType));
    
    // Different pointer types should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int32PtrType, int64PtrType));
    
    // But should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int32PtrType, int64PtrType));
}

TEST_F(ConversionConstraintTest, PointerToInteger)
{
    // Pointer to integer should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int32PtrType, int64Type));
    
    // But should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int32PtrType, int64Type));
}

TEST_F(ConversionConstraintTest, IntegerToPointer)
{
    // Integer to pointer should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int64Type, int32PtrType));
    
    // But should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int64Type, int32PtrType));
}

// Test enum conversions
TEST_F(ConversionConstraintTest, EnumToInteger)
{
    // Enum to integer should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(enumType, int32Type));
    
    // But should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(enumType, int32Type));
}

TEST_F(ConversionConstraintTest, IntegerToEnum)
{
    // Integer to enum should NOT be allowed implicitly
    EXPECT_FALSE(testImplicitConversion(int32Type, enumType));
    
    // But should be allowed explicitly
    EXPECT_TRUE(testExplicitConversion(int32Type, enumType));
}

// Test type variable conversions
TEST_F(ConversionConstraintTest, TypeVariableConversions)
{
    // Type variables should always allow conversion
    EXPECT_TRUE(testImplicitConversion(typeVar, int32Type));
    EXPECT_TRUE(testImplicitConversion(int32Type, typeVar));
    EXPECT_TRUE(testImplicitConversion(typeVar, typeVar));
    
    // Same for explicit conversions
    EXPECT_TRUE(testExplicitConversion(typeVar, int32Type));
    EXPECT_TRUE(testExplicitConversion(int32Type, typeVar));
    EXPECT_TRUE(testExplicitConversion(typeVar, typeVar));
}

// Test same type conversions
TEST_F(ConversionConstraintTest, SameTypeConversions)
{
    // Same types should always be valid
    EXPECT_TRUE(testImplicitConversion(int32Type, int32Type));
    EXPECT_TRUE(testImplicitConversion(float32Type, float32Type));
    EXPECT_TRUE(testImplicitConversion(int32PtrType, int32PtrType));
    EXPECT_TRUE(testImplicitConversion(boolType, boolType));
    
    // Same for explicit conversions
    EXPECT_TRUE(testExplicitConversion(int32Type, int32Type));
    EXPECT_TRUE(testExplicitConversion(float32Type, float32Type));
    EXPECT_TRUE(testExplicitConversion(int32PtrType, int32PtrType));
    EXPECT_TRUE(testExplicitConversion(boolType, boolType));
}

// Test invalid conversions
TEST_F(ConversionConstraintTest, InvalidConversions)
{
    // Integer to float should not be allowed
    EXPECT_FALSE(testImplicitConversion(int32Type, float32Type));
    EXPECT_FALSE(testExplicitConversion(int32Type, float32Type));
    
    // Float to integer should not be allowed
    EXPECT_FALSE(testImplicitConversion(float32Type, int32Type));
    EXPECT_FALSE(testExplicitConversion(float32Type, int32Type));
    
    // Integer to bool should not be allowed
    EXPECT_FALSE(testImplicitConversion(int32Type, boolType));
    EXPECT_FALSE(testExplicitConversion(int32Type, boolType));
    
    // Bool to integer should not be allowed
    EXPECT_FALSE(testImplicitConversion(boolType, int32Type));
    EXPECT_FALSE(testExplicitConversion(boolType, int32Type));
}

// Test that conversion constraints can be created and applied
TEST_F(ConversionConstraintTest, ConversionConstraintApplication)
{
    // Create some test expressions
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int8Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(8, 42), int8Type, loc
    );
    auto *int32Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(32, 100), int32Type, loc
    );
    
    // Create a conversion constraint: int8 -> int32 (should succeed)
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), int8Type, int32Type, int8Expr
    );
    
    SystemState state;
    auto result = cs->applyConversion(conversionConstraint, state);
    
    // The conversion should be applied successfully
    EXPECT_EQ(result, ConstraintResult::Applied);
}

TEST_F(ConversionConstraintTest, FailingConversionConstraintApplication)
{
    // Create some test expressions
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int64Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(64, 42), int64Type, loc
    );
    auto *int32Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(32, 100), int32Type, loc
    );
    
    // Create a conversion constraint: int64 -> int32 (should fail implicitly)
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), int64Type, int32Type, int64Expr
    );
    
    SystemState state;
    auto result = cs->applyConversion(conversionConstraint, state);
    
    // The conversion should fail
    EXPECT_EQ(result, ConstraintResult::Failed);
}

TEST_F(ConversionConstraintTest, CheckedCastConstraintApplication)
{
    // Create some test expressions
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int64Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(64, 42), int64Type, loc
    );
    auto *int32Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(32, 100), int32Type, loc
    );
    
    // Create a checked cast constraint: int64 -> int32 (should succeed)
    auto *checkedCastConstraint = Constraint::createCheckedCast(
        cs->getAllocator(), int64Type, int32Type, int64Expr
    );
    
    SystemState state;
    auto result = cs->applyCheckedCast(checkedCastConstraint, state);
    
    // The checked cast should be applied successfully
    EXPECT_EQ(result, ConstraintResult::Applied);
}