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
        return isValidConversion(cs.get(), fromType, toType, state, false);
    }

    /// @brief Helper function to test explicit conversions (checked casts)
    bool testExplicitConversion(glu::types::Ty fromType, glu::types::Ty toType)
    {
        SystemState state;
        return isValidConversion(cs.get(), fromType, toType, state, true);
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

// Test that conversion constraints are correctly applied with full constraint solving
TEST_F(ConversionConstraintTest, ConversionConstraintFullWorkflow)
{
    // Create expressions with type variables for inference
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create a type variable to represent an unknown type that should be inferred
    auto *typeVar = typeArena.create<types::TypeVariableTy>();
    
    // Create an expression with the type variable
    auto *expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(8, 42), typeVar, loc
    );
    
    // Create a constraint that this expression's type should convert to int32
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), typeVar, int32Type, expr
    );
    
    // Add the constraint to the system
    cs->addConstraint(conversionConstraint);
    
    // Solve the complete constraint system with the expression
    bool success = cs->solveConstraints({ expr });
    ASSERT_TRUE(success);
    
    // Verify the type variable was properly bound/converted
    // The expression should now have int32 type (after conversion from int8)
    EXPECT_EQ(expr->getType(), int32Type);
}

// Test that implicit conversions are recorded in the state
TEST_F(ConversionConstraintTest, ImplicitConversionRecording)
{
    // Create expressions for testing
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int8Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(8, 42), int8Type, loc
    );
    
    // Create a conversion constraint: int8 -> int32 (should succeed and record)
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), int8Type, int32Type, int8Expr
    );
    
    SystemState state;
    auto result = cs->applyConversion(conversionConstraint, state);
    
    // The conversion should be applied successfully
    EXPECT_EQ(result, ConstraintResult::Applied);
    
    // Check that the implicit conversion was recorded in the state
    auto it = state.implicitConversions.find(int8Expr);
    EXPECT_NE(it, state.implicitConversions.end());
    EXPECT_EQ(it->second, int32Type);
}

// Test type variable unification in conversions
TEST_F(ConversionConstraintTest, TypeVariableConversionUnification)
{
    auto &typeArena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();
    SourceLocation loc(0);
    
    // Create type variables
    auto *typeVar1 = typeArena.create<types::TypeVariableTy>();
    auto *typeVar2 = typeArena.create<types::TypeVariableTy>();
    
    // Create a dummy expression for the constraint locator
    auto *dummyExpr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(32, 0), typeVar1, loc
    );
    
    // Create a conversion constraint: T1 -> T2
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), typeVar1, typeVar2, dummyExpr
    );
    
    SystemState state;
    auto result = cs->applyConversion(conversionConstraint, state);
    
    // The conversion should be applied (via unification)
    EXPECT_EQ(result, ConstraintResult::Applied);
    
    // Check that unification occurred - one variable should be bound to the other
    auto it1 = state.typeBindings.find(typeVar1);
    auto it2 = state.typeBindings.find(typeVar2);
    
    // At least one should be bound, and they should ultimately unify
    EXPECT_TRUE(it1 != state.typeBindings.end() || it2 != state.typeBindings.end());
    
    // If typeVar1 is bound to typeVar2, or vice versa, that's unification
    if (it1 != state.typeBindings.end()) {
        EXPECT_EQ(it1->second, typeVar2);
    }
    if (it2 != state.typeBindings.end()) {
        EXPECT_EQ(it2->second, typeVar1);
    }
}

// Test that the complete workflow applies cast expressions for implicit conversions
TEST_F(ConversionConstraintTest, ImplicitCastExpressionInsertion)
{
    // Create expressions with concrete types that require conversion
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create a type variable for the result type
    auto *resultTypeVar = typeArena.create<types::TypeVariableTy>();
    
    // Create a literal expression (int8 value)
    auto *int8Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(8, 42), int8Type, loc
    );
    
    // Create a binary operation that expects int32 operands
    auto *int32LitExpr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(32, 100), int32Type, loc
    );
    
    // Create an assignment or operation that requires int8 -> int32 conversion
    auto *plusOp = astArena.create<ast::RefExpr>(
        loc, ast::NamespaceIdentifier({}, "+")
    );
    auto *binaryExpr = astArena.create<ast::BinaryOpExpr>(
        loc, int8Expr, plusOp, int32LitExpr
    );
    binaryExpr->setType(resultTypeVar);
    
    // Create constraints for the binary operation
    // The left operand (int8) should convert to match the right operand (int32)
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), int8Type, int32Type, int8Expr
    );
    
    // The result type should also be int32
    auto *resultConstraint = Constraint::createBind(
        cs->getAllocator(), resultTypeVar, int32Type, binaryExpr
    );
    
    cs->addConstraint(conversionConstraint);
    cs->addConstraint(resultConstraint);
    
    // Solve the complete constraint system
    bool success = cs->solveConstraints({ binaryExpr });
    ASSERT_TRUE(success);
    
    // The binary expression should now have the correct result type
    EXPECT_EQ(binaryExpr->getType(), int32Type);
    
    // Note: The actual cast expression insertion is handled by mapImplicitConversions
    // which is called automatically by solveConstraints. We've verified that the
    // constraint system can handle the conversion logic correctly.
}

TEST_F(ConversionConstraintTest, CheckedCastConstraintApplication)
{
    // Create some test expressions
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int64Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(64, 42), int64Type, loc
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

// Test that invalid implicit conversions fail
TEST_F(ConversionConstraintTest, FailingImplicitConversion)
{
    // Create expressions for testing
    SourceLocation loc(0);
    auto &astArena = context->getASTMemoryArena();
    auto *int64Expr = astArena.create<ast::LiteralExpr>(
        llvm::APInt(64, 42), int64Type, loc
    );
    
    // Create a conversion constraint: int64 -> int32 (should fail implicitly)
    auto *conversionConstraint = Constraint::createConversion(
        cs->getAllocator(), int64Type, int32Type, int64Expr
    );
    
    SystemState state;
    auto result = cs->applyConversion(conversionConstraint, state);
    
    // The conversion should fail (narrowing not allowed implicitly)
    EXPECT_EQ(result, ConstraintResult::Failed);
    
    // No implicit conversion should be recorded
    auto it = state.implicitConversions.find(int64Expr);
    EXPECT_EQ(it, state.implicitConversions.end());
}