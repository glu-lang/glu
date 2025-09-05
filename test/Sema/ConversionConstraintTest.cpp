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
        llvm::SmallVector<glu::ast::FieldDecl *> emptyFields;
        auto enumDecl = context->getASTMemoryArena().create<glu::ast::EnumDecl>(
            *context, SourceLocation(0), nullptr, "TestEnum", emptyFields
        );
        enumType = enumDecl->getType();
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

// Test nested type variable conversions - pointer to type variable
TEST_F(ConversionConstraintTest, NestedTypeVariablePointerConversion)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *elemTypeVar = typeArena.create<types::TypeVariableTy>();
    auto *targetTypeVar = typeArena.create<types::TypeVariableTy>();
    
    // Create pointer types: int32* and T*
    auto *int32PtrType = typeArena.create<types::PointerTy>(int32Type);
    auto *typeVarPtrType = typeArena.create<types::PointerTy>(elemTypeVar);
    auto *targetPtrType = typeArena.create<types::PointerTy>(targetTypeVar);
    
    // Test pointer to type variable conversion - should unify the pointee types
    SystemState state;
    
    // int32* -> T* should work by unifying int32 with T
    EXPECT_TRUE(cs->isValidConversion(int32PtrType, typeVarPtrType, state, false));
    
    // Verify that the type variable was bound to int32
    auto it = state.typeBindings.find(elemTypeVar);
    EXPECT_NE(it, state.typeBindings.end());
    EXPECT_EQ(it->second, int32Type);
    
    // Test the reverse: T* -> S* should unify T with S
    SystemState state2;
    EXPECT_TRUE(cs->isValidConversion(typeVarPtrType, targetPtrType, state2, false));
    
    // Both type variables should be unified
    auto it1 = state2.typeBindings.find(elemTypeVar);
    auto it2 = state2.typeBindings.find(targetTypeVar);
    // At least one should be bound, and they should be compatible
    EXPECT_TRUE(it1 != state2.typeBindings.end() || it2 != state2.typeBindings.end());
}

// Test nested type variable conversions - array to pointer
TEST_F(ConversionConstraintTest, NestedTypeVariableArrayToPointerConversion)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *elemTypeVar = typeArena.create<types::TypeVariableTy>();
    auto *targetTypeVar = typeArena.create<types::TypeVariableTy>();
    
    // Create types: int32[5] and T*
    auto *int32ArrayType = typeArena.create<types::StaticArrayTy>(int32Type, 5);
    auto *typeVarPtrType = typeArena.create<types::PointerTy>(elemTypeVar);

    // Test array to type variable pointer conversion
    SystemState state;
    
    // int32[5] -> T* should work by unifying int32 with T
    EXPECT_TRUE(cs->isValidConversion(int32ArrayType, typeVarPtrType, state, false));
    
    // Verify that the type variable was bound to int32
    auto it = state.typeBindings.find(elemTypeVar);
    EXPECT_NE(it, state.typeBindings.end());
    EXPECT_EQ(it->second, int32Type);
    
    // Test with type variable array to pointer
    auto *typeVarArrayType = typeArena.create<types::StaticArrayTy>(targetTypeVar, 3);
    SystemState state2;
    
    // T[3] -> S* should unify T with S
    EXPECT_TRUE(cs->isValidConversion(typeVarArrayType, typeVarPtrType, state2, false));
}

// Test nested type variable conversions - function types
TEST_F(ConversionConstraintTest, NestedTypeVariableFunctionConversion)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *retTypeVar = typeArena.create<types::TypeVariableTy>();
    auto *paramTypeVar = typeArena.create<types::TypeVariableTy>();
    
    // Create function types: (int32) -> int64 and (T) -> S
    std::vector<types::TypeBase *> int32Params = { int32Type };
    std::vector<types::TypeBase *> typeVarParams = { paramTypeVar };
    
    auto *concreteFuncType = typeArena.create<types::FunctionTy>(int32Params, int64Type);
    auto *typeVarFuncType = typeArena.create<types::FunctionTy>(typeVarParams, retTypeVar);
    
    // Test function type variable conversion
    SystemState state;
    
    // (int32) -> int64 converted to (T) -> S should unify appropriately
    EXPECT_TRUE(cs->isValidConversion(concreteFuncType, typeVarFuncType, state, false));
    
    // At least one type variable should be bound
    auto retIt = state.typeBindings.find(retTypeVar);
    auto paramIt = state.typeBindings.find(paramTypeVar);
    EXPECT_TRUE(retIt != state.typeBindings.end() || paramIt != state.typeBindings.end());
}

// Test nested type variable conversions - complex nested structures
TEST_F(ConversionConstraintTest, NestedTypeVariableComplexConversion)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *innerTypeVar = typeArena.create<types::TypeVariableTy>();

    // Create complex nested types: int32** (pointer to pointer to int32) and T** 
    auto *int32PtrType = typeArena.create<types::PointerTy>(int32Type);
    auto *int32PtrPtrType = typeArena.create<types::PointerTy>(int32PtrType);
    
    auto *innerPtrType = typeArena.create<types::PointerTy>(innerTypeVar);
    auto *outerPtrType = typeArena.create<types::PointerTy>(innerPtrType);
    
    // Test deeply nested pointer conversion
    SystemState state;
    
    // int32** -> T** should work by unifying the nested structures
    EXPECT_TRUE(cs->isValidConversion(int32PtrPtrType, outerPtrType, state, false));
    
    // The unification should establish proper type relationships
    // Note: The exact binding depends on the unification implementation,
    // but the conversion should succeed
}

// Test explicit conversions with nested type variables
TEST_F(ConversionConstraintTest, NestedTypeVariableExplicitConversion)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *typeVar1 = typeArena.create<types::TypeVariableTy>();
    auto *typeVar2 = typeArena.create<types::TypeVariableTy>();
    
    // Create pointer types with different concrete pointees: int32* and int64*
    auto *int32PtrType = typeArena.create<types::PointerTy>(int32Type);
    auto *int64PtrType = typeArena.create<types::PointerTy>(int64Type);
    auto *typeVar1PtrType = typeArena.create<types::PointerTy>(typeVar1);
    auto *typeVar2PtrType = typeArena.create<types::PointerTy>(typeVar2);
    
    // Test explicit pointer conversions with type variables
    SystemState state;
    
    // int32* -> T* should work explicitly and bind the type variable
    EXPECT_TRUE(cs->isValidConversion(int32PtrType, typeVar1PtrType, state, true));
    
    // T* -> S* should work explicitly and establish unification
    SystemState state2;
    EXPECT_TRUE(cs->isValidConversion(typeVar1PtrType, typeVar2PtrType, state2, true));
    
    // Mixed explicit conversions should work
    SystemState state3;
    EXPECT_TRUE(cs->isValidConversion(int32PtrType, int64PtrType, state3, true));
}

// Test edge cases with type variables in conversions
TEST_F(ConversionConstraintTest, NestedTypeVariableEdgeCases)
{
    auto &typeArena = context->getTypesMemoryArena();
    
    // Create type variables
    auto *typeVar = typeArena.create<types::TypeVariableTy>();
    
    // Test type variable to concrete type conversions in various contexts
    SystemState state;
    
    // Type variable should convert to any concrete type
    EXPECT_TRUE(cs->isValidConversion(typeVar, int32Type, state, false));
    EXPECT_TRUE(cs->isValidConversion(typeVar, int32Type, state, true));
    
    // Concrete type should convert to type variable (through unification)
    SystemState state2;
    EXPECT_TRUE(cs->isValidConversion(int32Type, typeVar, state2, false));
    
    // Test with enum conversions
    SystemState state3;
    auto *enumPtrType = typeArena.create<types::PointerTy>(enumType);
    auto *typeVarPtrType = typeArena.create<types::PointerTy>(typeVar);
    
    // enum* -> T* should work by unifying enum with T
    EXPECT_TRUE(cs->isValidConversion(enumPtrType, typeVarPtrType, state3, false));
}
