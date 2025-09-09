#include "AST/ASTContext.hpp"
#include "AST/Decl/ModuleDecl.hpp"
#include "AST/Expr/LiteralExpr.hpp"
#include "AST/Expr/StructMemberExpr.hpp"
#include "AST/Types/EnumTy.hpp"
#include "AST/Types/FloatTy.hpp"
#include "AST/Types/FunctionTy.hpp"
#include "AST/Types/IntTy.hpp"
#include "AST/Types/PointerTy.hpp"
#include "AST/Types/StaticArrayTy.hpp"
#include "AST/Types/StructTy.hpp"
#include "AST/Types/TypeVariableTy.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"
#include "Sema/ScopeTable.hpp"
#include "gtest/gtest.h"

using namespace glu;
using namespace glu::sema;
using namespace glu::types;

class AdvancedConstraintTest : public ::testing::Test {
protected:
    std::unique_ptr<ast::ASTContext> context;
    std::unique_ptr<SourceManager> sourceManager;
    std::unique_ptr<DiagnosticManager> diagManager;
    std::unique_ptr<ScopeTable> scopeTable;
    std::unique_ptr<ConstraintSystem> cs;
    ast::ModuleDecl *moduleDecl;
    llvm::BumpPtrAllocator
        allocator; // Member allocator for proper lifetime management

    // Types for testing
    IntTy *intType;
    FloatTy *floatType;
    TypeVariableTy *typeVar1;
    TypeVariableTy *typeVar2;
    TypeVariableTy *typeVar3;

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

        auto &arena = context->getTypesMemoryArena();
        intType = arena.create<IntTy>(IntTy::Signed, 32);
        floatType = arena.create<FloatTy>(32);
        typeVar1 = arena.create<TypeVariableTy>();
        typeVar2 = arena.create<TypeVariableTy>();
        typeVar3 = arena.create<TypeVariableTy>();

        cs->addTypeVariable(typeVar1);
        cs->addTypeVariable(typeVar2);
        cs->addTypeVariable(typeVar3);
    }

    ast::LiteralExpr *createMockExpr()
    {
        return context->getASTMemoryArena().create<ast::LiteralExpr>(
            llvm::APInt(32, 0), intType, SourceLocation::invalid
        );
    }
};

// Test function type unification
TEST_F(AdvancedConstraintTest, FunctionTypeUnification)
{
    auto &arena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();

    // Create function types: (Int) -> Float and (T1) -> T2
    std::vector<TypeBase *> params1 = { intType };
    std::vector<TypeBase *> params2 = { typeVar1 };

    auto *funcType1 = arena.create<FunctionTy>(params1, floatType);
    auto *funcType2 = arena.create<FunctionTy>(params2, typeVar2);

    // Create a mock function call expression with the function type
    auto *funcExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "func")
    );
    funcExpr->setType(funcType2); // Function has generic type (T1) -> T2

    // Create equal constraint between the function types
    // FIXME: this should work with conversion constraints
    auto *equalConstraint = Constraint::createEqual(
        cs->getAllocator(), funcType1, funcType2, funcExpr
    );
    cs->addConstraint(equalConstraint);

    // Initially function expression has generic type (T1) -> T2
    EXPECT_EQ(funcExpr->getType(), funcType2);

    // Solve constraints and apply type mappings - this should unify the
    // function types
    bool success = cs->solveConstraints({ funcExpr });
    ASSERT_TRUE(success);

    // The function expression type should now be concrete (Int) -> Float
    EXPECT_EQ(funcExpr->getType(), funcType1);
}

// Test array type unification
TEST_F(AdvancedConstraintTest, ArrayTypeUnification)
{
    auto &arena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();

    // Create array types: Int[10] and T1[10]
    auto *arrayType1 = arena.create<StaticArrayTy>(intType, 10);
    auto *arrayType2 = arena.create<StaticArrayTy>(typeVar1, 10);

    // Create an array expression with the generic type
    auto *arrayExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "arr")
    );
    arrayExpr->setType(arrayType2); // Array has generic element type T1[10]

    // Create equal constraint between the array types
    auto *equalConstraint = Constraint::createEqual(
        cs->getAllocator(), arrayType1, arrayType2, arrayExpr
    );
    cs->addConstraint(equalConstraint);

    // Initially array expression has generic type T1[10]
    EXPECT_EQ(arrayExpr->getType(), arrayType2);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ arrayExpr });
    ASSERT_TRUE(success);

    // Verify that T1 was bound to Int through array type unification
    // The array expression type should now be concrete Int[10]
    EXPECT_EQ(arrayExpr->getType(), arrayType1);
}

// Test array size mismatch
TEST_F(AdvancedConstraintTest, ArraySizeMismatch)
{
    auto &arena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();

    // Create array types with different sizes: Int[10] and Int[20]
    auto *arrayType1 = arena.create<StaticArrayTy>(intType, 10);
    auto *arrayType2 = arena.create<StaticArrayTy>(intType, 20);

    // Create an array expression
    auto *arrayExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "mismatchedArr")
    );
    arrayExpr->setType(arrayType2); // Array has type Int[20]

    // Create equal constraint between arrays with different sizes
    auto *equalConstraint = Constraint::createEqual(
        cs->getAllocator(), arrayType1, arrayType2, arrayExpr
    );
    cs->addConstraint(equalConstraint);

    // Solve constraints - should fail due to size mismatch
    bool success = cs->solveConstraints({ arrayExpr });
    EXPECT_FALSE(success); // Constraint solving should fail
}

TEST_F(AdvancedConstraintTest, DefaultableExplored)
{
    auto &astArena = context->getASTMemoryArena();

    // Create an expression with a type variable
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "defaulted")
    );
    expr->setType(typeVar1);

    // Create a defaultable constraint binding T1 to Int
    auto *bindConstraint = Constraint::createDefaultable(
        cs->getAllocator(), typeVar1, intType, expr
    );

    // Initially expression has type variable
    EXPECT_EQ(expr->getType(), typeVar1);

    // Add the constraint to the system
    cs->addConstraint(bindConstraint);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ expr });
    ASSERT_TRUE(success);

    // Verify that T1 was bound to Int
    EXPECT_EQ(expr->getType(), intType);
}

TEST_F(AdvancedConstraintTest, DefaultableIgnored)
{
    auto &astArena = context->getASTMemoryArena();

    // Create an expression with a type variable
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "defaulted")
    );
    expr->setType(typeVar1);

    // Create a defaultable constraint binding T1 to Int
    auto *defaultableConstraint = Constraint::createDefaultable(
        cs->getAllocator(), typeVar1, intType, expr
    );
    cs->addConstraint(defaultableConstraint);

    // Create a defaultable constraint binding T1 to Int
    auto *bindConstraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, floatType, expr);
    cs->addConstraint(bindConstraint);

    // Initially expression has type variable
    EXPECT_EQ(expr->getType(), typeVar1);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ expr });
    ASSERT_TRUE(success);

    // Verify that T1 was bound to Float
    EXPECT_EQ(expr->getType(), floatType);
}

// Test bind-to-pointer-type constraint
TEST_F(AdvancedConstraintTest, BindToPointerTypeConstraint)
{
    auto &arena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();

    // Create *Int
    auto *ptrType = arena.create<PointerTy>(intType);

    // Create an expression with a type variable
    auto *ptrExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "ptr")
    );
    ptrExpr->setType(typeVar1);

    // Test bind-to-pointer-type constraint: typeVar1 should be bound to *Int
    // The constraint means: intType is the element type of typeVar1 (which
    // should be *Int)
    auto *equalConstraint = Constraint::createBindToPointerType(
        cs->getAllocator(), intType, typeVar1, ptrExpr
    );

    // Initially expression has type variable
    EXPECT_EQ(ptrExpr->getType(), typeVar1);

    // Add the constraint to the system
    cs->addConstraint(equalConstraint);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ ptrExpr });
    ASSERT_TRUE(success);

    // Verify that T1 was bound to *Int
    EXPECT_EQ(ptrExpr->getType(), ptrType);
}

// Test actual disjunction solving
TEST_F(AdvancedConstraintTest, ComplexDisjunctionSolving)
{
    auto &astArena = context->getASTMemoryArena();

    // Create an expression that could be either Int or Float
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "ambiguous")
    );
    expr->setType(typeVar1);

    // Initially expression has type variable
    EXPECT_EQ(expr->getType(), typeVar1);

    // Create two alternative constraints: T1 = Int OR T1 = Float
    auto *intConstraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, expr);
    auto *floatConstraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, floatType, expr);

    // Create a disjunction: (T1 = Int) OR (T1 = Float)
    llvm::SmallVector<Constraint *, 2> alternatives
        = { intConstraint, floatConstraint };
    auto *disjunction = Constraint::createDisjunction(
        cs->getAllocator(), alternatives, expr, true
    );

    // Add additional constraint to force a specific choice: T1 = Int
    auto *forcingConstraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, expr);

    cs->addConstraint(disjunction);
    cs->addConstraint(forcingConstraint);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ expr });
    ASSERT_TRUE(success);

    // The forcing constraint should cause T1 to be bound to Int
    EXPECT_EQ(expr->getType(), intType);
}

// Test occurs check with complex types
TEST_F(AdvancedConstraintTest, ComplexOccursCheck)
{
    auto &arena = context->getTypesMemoryArena();
    auto &astArena = context->getASTMemoryArena();

    // Create an expression with a type variable
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "recursive")
    );
    expr->setType(typeVar1);

    // Create T1 -> *T1 (function taking T1 and returning pointer to T1)
    std::vector<TypeBase *> params = { typeVar1 };
    auto *ptrToTypeVar = arena.create<PointerTy>(typeVar1);
    auto *funcType = arena.create<FunctionTy>(params, ptrToTypeVar);

    // Try to create equal constraint T1 = (T1) -> *T1 (should be detectable by
    // solver)
    auto *equalConstraint
        = Constraint::createEqual(cs->getAllocator(), typeVar1, funcType, expr);
    cs->addConstraint(equalConstraint);

    // Initially expression has type variable
    EXPECT_EQ(expr->getType(), typeVar1);

    // Solve constraints - solver should detect the occurs check violation and
    // fail
    bool success = cs->solveConstraints({ expr });
    EXPECT_FALSE(
        success
    ); // Should fail due to occurs check (infinite type T1 = (T1) -> *T1)

    // Test that constraint was created successfully
    EXPECT_EQ(equalConstraint->getKind(), ConstraintKind::Equal);

    // Expression should still have the original type variable since solving
    // failed
    EXPECT_EQ(expr->getType(), typeVar1);
}

// Test conjunction constraint solving
TEST_F(AdvancedConstraintTest, ConjunctionConstraintSolving)
{
    auto &astArena = context->getASTMemoryArena();

    // Create an expression where multiple constraints must all be satisfied
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "conjunctive")
    );
    expr->setType(typeVar1);

    // Initially expression has type variable
    EXPECT_EQ(expr->getType(), typeVar1);

    // Create multiple constraints that must all be satisfied: T1 = Int AND T1 =
    // Int
    auto *constraint1
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, expr);
    auto *constraint2
        = Constraint::createEqual(cs->getAllocator(), typeVar1, intType, expr);

    // Create a conjunction: (T1 = Int) AND (T1 = Int)
    llvm::SmallVector<Constraint *, 2> requirements
        = { constraint1, constraint2 };
    auto *conjunction
        = Constraint::createConjunction(cs->getAllocator(), requirements, expr);

    cs->addConstraint(conjunction);

    // Solve constraints and apply type mappings
    bool success = cs->solveConstraints({ expr });
    ASSERT_TRUE(success);

    // Both constraints should be satisfied, T1 should be bound to Int
    EXPECT_EQ(expr->getType(), intType);
}
