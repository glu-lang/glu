#include "Sema/ConstraintSystem.hpp"
#include "AST/ASTContext.hpp"
#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ScopeTable.hpp"
#include "Sema/TyMapperVisitor.hpp"
#include "gtest/gtest.h"

using namespace glu;
using namespace glu::sema;
using namespace glu::types;

class ConstraintSystemTest : public ::testing::Test {
protected:
    std::unique_ptr<ast::ASTContext> context;
    std::unique_ptr<SourceManager> sourceManager;
    std::unique_ptr<DiagnosticManager> diagManager;
    std::unique_ptr<ScopeTable> scopeTable;
    std::unique_ptr<ConstraintSystem> cs;
    llvm::BumpPtrAllocator
        allocator; // Member allocator to ensure it stays alive
    ast::ModuleDecl *moduleDecl;

    // Common types for testing
    IntTy *intType;
    FloatTy *floatType;
    BoolTy *boolType;
    TypeVariableTy *typeVar1;
    TypeVariableTy *typeVar2;
    TypeVariableTy *typeVar3;

    // NOTE: In these tests, we create standalone expressions that are NOT part
    // of the module tree. We use the new solveConstraints(expressions) API that
    // automatically applies type mappings to the provided expressions after
    // solving constraints. This eliminates the need for manual calls to
    // getBestSolution() and mapTypeVariablesToExpressions().

    void SetUp() override
    {
        sourceManager = std::make_unique<SourceManager>();
        diagManager = std::make_unique<DiagnosticManager>(*sourceManager);
        context = std::make_unique<ast::ASTContext>();

        // Create a simple module declaration for ScopeTable
        SourceLocation loc(0);
        llvm::ArrayRef<ast::DeclBase *> emptyDecls;
        moduleDecl = ast::ModuleDecl::create(
            allocator, loc, emptyDecls, context.get()
        );

        scopeTable = std::make_unique<ScopeTable>(moduleDecl);
        cs = std::make_unique<ConstraintSystem>(
            scopeTable.get(), *diagManager, context.get()
        );

        // Create common types
        auto &arena = context->getTypesMemoryArena();
        intType = arena.create<IntTy>(IntTy::Signed, 32);
        floatType = arena.create<FloatTy>(32);
        boolType = arena.create<BoolTy>();
        typeVar1 = arena.create<TypeVariableTy>();
        typeVar2 = arena.create<TypeVariableTy>();
        typeVar3 = arena.create<TypeVariableTy>();

        cs->addTypeVariable(typeVar1);
        cs->addTypeVariable(typeVar2);
        cs->addTypeVariable(typeVar3);
    }

    ast::LiteralExpr *createIntLiteral(int value, TypeBase *type = nullptr)
    {
        if (!type)
            type = intType;
        return context->getASTMemoryArena().create<ast::LiteralExpr>(
            llvm::APInt(32, value), type, SourceLocation::invalid
        );
    }
};

// Test: Variable declaration with type inference
// Scenario: let x = 42; (x should infer to Int)
TEST_F(ConstraintSystemTest, VariableDeclarationTypeInference)
{
    auto &astArena = context->getASTMemoryArena();

    // Create literal expression: 42 (already has concrete type Int)
    auto *literalExpr = createIntLiteral(42);
    EXPECT_EQ(literalExpr->getType(), intType);

    // Create variable reference with type variable: let x: T1 = 42
    auto *varRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    varRef->setType(typeVar1); // Initially unknown type
    EXPECT_EQ(varRef->getType(), typeVar1);

    // Add constraint: T1 = Int (variable should have same type as literal)
    auto *constraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, varRef);
    cs->addConstraint(constraint);

    // Solve constraints and apply type mappings to test expressions
    bool success = cs->solveConstraints({ varRef });
    ASSERT_TRUE(success);

    // Verify constraint was processed
    EXPECT_EQ(cs->getConstraints().size(), 1);
    EXPECT_EQ(cs->getConstraints()[0]->getKind(), ConstraintKind::Bind);

    // Now verify the actual type mapping:
    // The type variable T1 should be bound to Int and
    // varRef->getType() should be updated from typeVar1 to intType
    EXPECT_EQ(varRef->getType(), intType);

    // This verifies the complete type inference workflow:
    // 1. Expression starts with TypeVariableTy (T1)
    // 2. Constraint is added (T1 = Int)
    // 3. solveConstraints() with expressions automatically applies the solution
    // 4. Expression type is updated to the inferred concrete type (Int)
    // 1. Expression starts with type variable: x: T1
    // 2. Constraint added: T1 = Int
    // 3. Solver binds: T1 -> Int
    // 4. Type mapper updates: x: Int
}

// Test: Binary operation type inference
// Scenario: let result = x + y; where x, y should infer to Int
TEST_F(ConstraintSystemTest, BinaryOperationTypeInference)
{
    auto &astArena = context->getASTMemoryArena();

    // Create expressions with type variables
    auto *xRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    xRef->setType(typeVar1); // x: T1

    auto *yRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "y")
    );
    yRef->setType(typeVar2); // y: T2

    auto *plusOp = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "+")
    );

    auto *binaryExpr = astArena.create<ast::BinaryOpExpr>(
        SourceLocation::invalid, xRef, plusOp, yRef
    );
    binaryExpr->setType(typeVar3); // result: T3

    // Initially all expressions have type variables
    EXPECT_EQ(xRef->getType(), typeVar1);
    EXPECT_EQ(yRef->getType(), typeVar2);
    EXPECT_EQ(binaryExpr->getType(), typeVar3);

    // Add constraints for integer addition:
    // 1. T1 = Int (x is Int)
    auto *constraint1
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, xRef);
    cs->addConstraint(constraint1);

    // 2. T2 = Int (y is Int)
    auto *constraint2
        = Constraint::createBind(cs->getAllocator(), typeVar2, intType, yRef);
    cs->addConstraint(constraint2);

    // 3. T3 = Int (result is Int for integer addition)
    auto *constraint3 = Constraint::createBind(
        cs->getAllocator(), typeVar3, intType, binaryExpr
    );
    cs->addConstraint(constraint3);

    // Solve constraints and apply type mappings to test expressions
    llvm::SmallVector<glu::ast::ExprBase *, 3> expressions
        = { xRef, yRef, binaryExpr };
    bool success = cs->solveConstraints(expressions);
    ASSERT_TRUE(success);

    // Verify constraints were processed
    EXPECT_EQ(cs->getConstraints().size(), 3);

    // Now verify the actual type mapping - TypeVariableTyMapper should have
    // updated types: All type variables should be bound to Int and expressions
    // updated accordingly
    EXPECT_EQ(xRef->getType(), intType);
    EXPECT_EQ(yRef->getType(), intType);
    EXPECT_EQ(binaryExpr->getType(), intType);

    // This demonstrates complete type inference for binary operations:
    // 1. Start: x: T1, y: T2, (x+y): T3 (all type variables)
    // 2. Constraints: T1 = Int, T2 = Int, T3 = Int
    // 3. Solution: T1 -> Int, T2 -> Int, T3 -> Int
    // 4. Final: x: Int, y: Int, (x+y): Int (all concrete types)
}

// Test: Function call type inference
// Scenario: let result = identity(42); where identity<T>(x: T) -> T
TEST_F(ConstraintSystemTest, FunctionCallTypeInference)
{
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();

    // Create generic function type: (T) -> T (where T is unbound)
    auto *genericT = typeArena.create<TypeVariableTy>();
    cs->addTypeVariable(genericT);

    std::vector<TypeBase *> genericParams = { genericT };
    auto *genericFuncType
        = typeArena.create<FunctionTy>(genericParams, genericT);

    // Create function reference: identity<T>
    auto *funcRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "identity")
    );
    funcRef->setType(genericFuncType);

    // Create argument: 42 (concrete Int type)
    auto *argExpr = createIntLiteral(42);
    EXPECT_EQ(argExpr->getType(), intType);

    // Create function call with type variable result
    auto *callExpr = astArena.create<ast::CallExpr>(
        SourceLocation::invalid, funcRef,
        llvm::SmallVector<ast::ExprBase *> { argExpr }
    );
    callExpr->setType(typeVar1); // result: T1 (unknown initially)

    EXPECT_EQ(callExpr->getType(), typeVar1);

    // Create expected function type based on actual call: (Int) -> T1
    // This represents what the function type should be after type inference
    std::vector<TypeBase *> expectedParams = { intType };
    auto *expectedFuncType
        = typeArena.create<FunctionTy>(expectedParams, typeVar1);

    // Add constraint for function type unification:
    // The generic function type (T -> T) should unify with the expected type
    // (Int -> T1) This tests the recursive unification system:
    // - Parameter types: T = Int
    // - Return types: T = T1
    // Therefore: T1 = Int (transitively)
    auto *funcTypeConstraint = Constraint::createEqual(
        cs->getAllocator(), genericFuncType, expectedFuncType, callExpr
    );
    cs->addConstraint(funcTypeConstraint);

    // Solve constraints and apply type mappings to test expressions
    bool success = cs->solveConstraints({ callExpr });
    ASSERT_TRUE(success);

    // Verify constraints were processed
    EXPECT_EQ(cs->getConstraints().size(), 1);

    // Verify the actual type mapping - recursive unification should have bound:
    // genericT = Int (from parameter unification T = Int)
    // typeVar1 = Int (from return type unification T = T1, and T = Int)
    EXPECT_EQ(callExpr->getType(), intType);

    // This demonstrates complete generic function type inference via recursive
    // unification:
    // 1. Start: identity<T>(42) -> T1 with generic type (T) -> T
    // 2. Expected type: (Int) -> T1 (based on actual argument and result types)
    // 3. Constraint: (T) -> T = (Int) -> T1 (structural equality constraint)
    // 4. Recursive unification via Equal constraint:
    //    - Parameters: T = Int
    //    - Return types: T = T1, therefore T1 = Int
    // 5. Final: identity<Int>(42) -> Int (all concrete types)
}

// Test: Type propagation chain
// Scenario: let x = 42; let y = x; let z = y; (all should infer to Int)
TEST_F(ConstraintSystemTest, TypePropagationChain)
{
    auto &astArena = context->getASTMemoryArena();

    // Create literal: 42 (concrete Int) - not used in this test directly
    // but demonstrates that concrete types exist alongside type variables
    createIntLiteral(42);

    // Create variable chain: x: T1, y: T2, z: T3
    auto *xRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    xRef->setType(typeVar1);

    auto *yRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "y")
    );
    yRef->setType(typeVar2);

    auto *zRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "z")
    );
    zRef->setType(typeVar3);

    // Initially all have type variables
    EXPECT_EQ(xRef->getType(), typeVar1);
    EXPECT_EQ(yRef->getType(), typeVar2);
    EXPECT_EQ(zRef->getType(), typeVar3);

    // Add constraint chain:
    // 1. T1 = Int (x = 42)
    auto *constraint1
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, xRef);
    cs->addConstraint(constraint1);

    // 2. T2 = T1 (y = x)
    auto *constraint2
        = Constraint::createBind(cs->getAllocator(), typeVar2, typeVar1, yRef);
    cs->addConstraint(constraint2);

    // 3. T3 = T2 (z = y)
    auto *constraint3
        = Constraint::createBind(cs->getAllocator(), typeVar3, typeVar2, zRef);
    cs->addConstraint(constraint3);

    // Solve constraints and apply type mappings to test expressions
    llvm::SmallVector<glu::ast::ExprBase *, 3> expressions
        = { xRef, yRef, zRef };
    bool success = cs->solveConstraints(expressions);
    ASSERT_TRUE(success);

    // Verify constraints were processed
    EXPECT_EQ(cs->getConstraints().size(), 3);

    // Verify the actual type mapping - all variables should be bound to Int:
    EXPECT_EQ(xRef->getType(), intType);
    EXPECT_EQ(yRef->getType(), intType);
    EXPECT_EQ(zRef->getType(), intType);

    // This demonstrates complete transitive type inference:
    // 1. Start: x: T1 = 42, y: T2 = x, z: T3 = y (type variables)
    // 2. Constraints: T1 = Int, T2 = T1, T3 = T2
    // 3. Solution: T1 -> Int, T2 -> Int, T3 -> Int (transitive resolution)
    // 4. Final: x: Int, y: Int, z: Int (all concrete types)
}

// Test: Conditional expression type inference
// Scenario: let result = condition ? x : y; where x, y should unify
TEST_F(ConstraintSystemTest, ConditionalExpressionTypeInference)
{
    auto &astArena = context->getASTMemoryArena();

    // Create condition expression
    auto *condExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "condition")
    );
    condExpr->setType(boolType); // condition is Bool

    // Create true/false expressions with type variables
    auto *trueExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    trueExpr->setType(typeVar1); // x: T1

    auto *falseExpr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "y")
    );
    falseExpr->setType(typeVar2); // y: T2

    auto *ternaryExpr = astArena.create<ast::TernaryConditionalExpr>(
        SourceLocation::invalid, condExpr, trueExpr, falseExpr
    );
    ternaryExpr->setType(typeVar3); // result: T3

    // Initially expressions have type variables
    EXPECT_EQ(trueExpr->getType(), typeVar1);
    EXPECT_EQ(falseExpr->getType(), typeVar2);
    EXPECT_EQ(ternaryExpr->getType(), typeVar3);

    // Add constraints for ternary expression:
    // 1. T1 = Int (x is Int)
    auto *constraint1 = Constraint::createBind(
        cs->getAllocator(), typeVar1, intType, trueExpr
    );
    cs->addConstraint(constraint1);

    // 2. T2 = T1 (y must have same type as x)
    auto *constraint2 = Constraint::createBind(
        cs->getAllocator(), typeVar2, typeVar1, falseExpr
    );
    cs->addConstraint(constraint2);

    // 3. T3 = T1 (result has same type as branches)
    auto *constraint3 = Constraint::createBind(
        cs->getAllocator(), typeVar3, typeVar1, ternaryExpr
    );
    cs->addConstraint(constraint3);

    // Solve constraints
    // Solve constraints and apply type mappings to test expressions
    llvm::SmallVector<glu::ast::ExprBase *, 3> expressions
        = { trueExpr, falseExpr, ternaryExpr };
    bool success = cs->solveConstraints(expressions);
    ASSERT_TRUE(success);

    // Verify constraints were processed
    EXPECT_EQ(cs->getConstraints().size(), 3);

    // Verify the actual type mapping - all expressions should be bound to Int:
    EXPECT_EQ(trueExpr->getType(), intType);
    EXPECT_EQ(falseExpr->getType(), intType);
    EXPECT_EQ(ternaryExpr->getType(), intType);

    // This demonstrates complete conditional expression type inference:
    // 1. Start: condition ? x: T1 : y: T2 -> result: T3 (type variables)
    // 2. Constraints: T1 = Int, T2 = T1, T3 = T1 (type unification)
    // 3. Solution: T1 -> Int, T2 -> Int, T3 -> Int
    // 4. Final: condition ? x: Int : y: Int -> Int (unified types)
}

// Test: Struct member access type inference
// Scenario: let member = obj.field; where obj: {field: Int}, member: T
TEST_F(ConstraintSystemTest, StructMemberAccessTypeInference)
{
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();

    // Create a struct type with a field: struct { field: Int }
    llvm::SmallVector<ast::FieldDecl *> fields
        = { astArena.create<ast::FieldDecl>(
            SourceLocation::invalid, "field", intType, nullptr
        ) };

    // First create a StructDecl
    auto *structDecl = ast::StructDecl::create(
        astArena.getAllocator(), *context, SourceLocation::invalid, nullptr,
        "TestStruct", fields
    );

    // Then create StructTy using the declaration
    auto *structType = typeArena.create<glu::types::StructTy>(structDecl);

    // Create object reference with concrete struct type
    auto *objRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "obj")
    );
    objRef->setType(structType); // obj: TestStruct

    // Create member access expression: obj.field
    auto *memberExpr = astArena.create<ast::StructMemberExpr>(
        SourceLocation::invalid, objRef, "field"
    );
    memberExpr->setType(typeVar2); // member: T2 (unknown initially)

    EXPECT_EQ(objRef->getType(), structType);
    EXPECT_EQ(memberExpr->getType(), typeVar2);

    // Add ValueMember constraint: obj.field has type T2 where obj: TestStruct
    // This constraint links the struct type to the member type through the
    // member access
    auto *valueMemberConstraint = Constraint::createMember(
        cs->getAllocator(), ConstraintKind::ValueMember, structType, typeVar2,
        memberExpr, memberExpr
    );
    cs->addConstraint(valueMemberConstraint);

    // Solve constraints and apply type mappings to test expressions
    bool success = cs->solveConstraints({ memberExpr });
    ASSERT_TRUE(success);

    // Verify constraints were processed
    EXPECT_EQ(cs->getConstraints().size(), 1);

    // Verify the actual type mapping - member expression should be bound to
    // Int:
    EXPECT_EQ(memberExpr->getType(), intType);

    // This demonstrates complete struct member access type inference:
    // 1. Start: obj.field -> T2 (type variable)
    // 2. Constraint: ValueMember(TestStruct, T2, obj.field)
    // 3. Constraint system finds field "field" in TestStruct has type Int
    // 4. Solution: T2 -> Int
    // 5. Final: obj.field -> Int (concrete type)
}

// Test: Complex expression with multiple constraints
// Scenario: let result = func(a + b, c); demonstrating multiple type
// relationships
TEST_F(ConstraintSystemTest, ComplexExpressionTypeInference)
{
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();

    // Create function type: (Int, Int) -> Float
    std::vector<TypeBase *> params = { intType, intType };
    auto *funcType = typeArena.create<FunctionTy>(params, floatType);

    auto *funcRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "func")
    );
    funcRef->setType(funcType);

    // Create expressions with type variables: a: T1, b: T2, c: T3
    auto *aRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "a")
    );
    aRef->setType(typeVar1);

    auto *bRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "b")
    );
    bRef->setType(typeVar2);

    auto *cRef = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "c")
    );
    cRef->setType(typeVar3);

    // Create binary expression: a + b
    auto *plusOp = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "+")
    );
    auto *addExpr = astArena.create<ast::BinaryOpExpr>(
        SourceLocation::invalid, aRef, plusOp, bRef
    );
    auto *addResultType = typeArena.create<TypeVariableTy>();
    cs->addTypeVariable(addResultType);
    addExpr->setType(addResultType);

    // Create function call: func(a + b, c)
    auto *callExpr = astArena.create<ast::CallExpr>(
        SourceLocation::invalid, funcRef,
        llvm::SmallVector<ast::ExprBase *> { addExpr, cRef }
    );
    auto *resultType = typeArena.create<TypeVariableTy>();
    cs->addTypeVariable(resultType);
    callExpr->setType(resultType);

    // Initially all have type variables
    EXPECT_EQ(aRef->getType(), typeVar1);
    EXPECT_EQ(bRef->getType(), typeVar2);
    EXPECT_EQ(cRef->getType(), typeVar3);
    EXPECT_EQ(addExpr->getType(), addResultType);
    EXPECT_EQ(callExpr->getType(), resultType);

    // Add sophisticated constraints for complex expression:

    // 1. Argument conversion constraints for binary addition (a + b)
    // These model how operators require compatible argument types
    auto *leftOpConstraint = Constraint::createConversion(
        cs->getAllocator(), typeVar1, intType, aRef
    );
    cs->addConstraint(leftOpConstraint);

    auto *rightOpConstraint = Constraint::createConversion(
        cs->getAllocator(), typeVar2, intType, bRef
    );
    cs->addConstraint(rightOpConstraint);

    // 2. Equal constraint for binary operation result type
    // Models that a + b has the same type as its operands for integer
    // arithmetic
    auto *addResultConstraint = Constraint::createEqual(
        cs->getAllocator(), addResultType, intType, addExpr
    );
    cs->addConstraint(addResultConstraint);

    // 3. Function call argument conversion constraints
    // These model how function arguments must be convertible to parameter types
    auto *firstArgConstraint = Constraint::createConversion(
        cs->getAllocator(), addResultType, intType, callExpr
    );
    cs->addConstraint(firstArgConstraint);

    auto *secondArgConstraint = Constraint::createConversion(
        cs->getAllocator(), typeVar3, intType, callExpr
    );
    cs->addConstraint(secondArgConstraint);

    // 4. Function type unification constraint
    // Models how the actual call type must unify with the declared function
    // type Expected call type: (Int, Int) -> resultType should equal (Int, Int)
    // -> Float
    std::vector<TypeBase *> actualParams = { intType, intType };
    auto *actualCallType
        = typeArena.create<FunctionTy>(actualParams, resultType);

    auto *funcUnificationConstraint = Constraint::createEqual(
        cs->getAllocator(), funcType, actualCallType, callExpr
    );
    cs->addConstraint(funcUnificationConstraint);

    // Solve constraints and apply type mappings to test expressions
    llvm::SmallVector<glu::ast::ExprBase *, 5> expressions
        = { aRef, bRef, cRef, addExpr, callExpr };
    bool success = cs->solveConstraints(expressions);
    ASSERT_TRUE(success);

    // Verify the actual type mapping - all expressions should have concrete
    // types:
    EXPECT_EQ(aRef->getType(), intType);
    EXPECT_EQ(bRef->getType(), intType);
    EXPECT_EQ(cRef->getType(), intType);
    EXPECT_EQ(addExpr->getType(), intType);
    EXPECT_EQ(callExpr->getType(), floatType);

    // This demonstrates sophisticated constraint system capabilities:
    // 1. Start: func(a: T1 + b: T2, c: T3) -> result: T5 (type variables)
    // 2. Advanced constraints:
    //    - Conversion: T1 -> Int, T2 -> Int (binary op arguments)
    //    - Equal: addResultType = Int (structural type equality)
    //    - Conversion: addResultType -> Int, T3 -> Int (function call
    //    arguments)
    //    - Equal: funcType = actualCallType (function type unification)
    // 3. Complex solving: multiple constraint types working together
    // 4. Final: func(a: Int + b: Int, c: Int) -> Float (all concrete types)
}

// Test: Error case - conflicting type constraints
// Scenario: T1 = Int AND T1 = Float (should fail)
TEST_F(ConstraintSystemTest, ConflictingConstraints)
{
    auto &astArena = context->getASTMemoryArena();

    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    expr->setType(typeVar1);

    // Add conflicting constraints
    auto *constraint1
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, expr);
    auto *constraint2
        = Constraint::createBind(cs->getAllocator(), typeVar1, floatType, expr);

    cs->addConstraint(constraint1);
    cs->addConstraint(constraint2);

    // Solve constraints - should handle the conflict
    bool success = cs->solveConstraints({ expr });
    ASSERT_FALSE(success); // Expect failure due to conflicting constraints

    // Check if the system detected the conflict

    // Constraints should be added, but solving may fail or produce no solutions
}

// Test: Occurs check prevention
// Scenario: T1 = *T1 (should be prevented)
TEST_F(ConstraintSystemTest, OccursCheckPrevention)
{
    auto &astArena = context->getASTMemoryArena();
    auto &typeArena = context->getTypesMemoryArena();

    // Create pointer to type variable: *T1
    auto *ptrType = typeArena.create<PointerTy>(typeVar1);

    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "recursive")
    );
    expr->setType(typeVar1);

    // Try to create: T1 = *T1 (should be prevented by occurs check)
    auto *constraint
        = Constraint::createBind(cs->getAllocator(), typeVar1, ptrType, expr);
    cs->addConstraint(constraint);

    // Solve constraints - occurs check should prevent infinite type
    bool success = cs->solveConstraints({ expr });
    // Occurs check should prevent the solution, so we expect failure
    EXPECT_FALSE(success);
}

// Test: Module auto-mapping functionality
// Demonstrates that solveConstraints() with no arguments automatically maps all
// module expressions
TEST_F(ConstraintSystemTest, ModuleExpressionAutoMapping)
{
    auto &astArena = context->getASTMemoryArena();

    // Create expressions that are part of module declarations
    auto *expr1 = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "value1")
    );
    expr1->setType(typeVar1);

    auto *expr2 = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "value2")
    );
    expr2->setType(typeVar2);

    // Create let declarations containing these expressions in a module
    auto *letDecl1 = astArena.create<ast::LetDecl>(
        SourceLocation::invalid, "x", typeVar1, expr1
    );
    auto *letDecl2 = astArena.create<ast::LetDecl>(
        SourceLocation::invalid, "y", typeVar2, expr2
    );

    // Create a new module with these declarations
    llvm::SmallVector<ast::DeclBase *> moduleDecls = { letDecl1, letDecl2 };
    auto *moduleWithExprs = ast::ModuleDecl::create(
        allocator, SourceLocation::invalid, moduleDecls, context.get()
    );

    // Create a new constraint system with the module containing expressions
    auto moduleScope = std::make_unique<ScopeTable>(moduleWithExprs);
    auto moduleCs = std::make_unique<ConstraintSystem>(
        moduleScope.get(), *diagManager, context.get()
    );
    moduleCs->addTypeVariable(typeVar1);
    moduleCs->addTypeVariable(typeVar2);

    // Add constraints for the module expressions
    auto *constraint1 = Constraint::createBind(
        moduleCs->getAllocator(), typeVar1, intType, expr1
    );
    auto *constraint2 = Constraint::createBind(
        moduleCs->getAllocator(), typeVar2, floatType, expr2
    );
    moduleCs->addConstraint(constraint1);
    moduleCs->addConstraint(constraint2);

    // Initially expressions have type variables
    EXPECT_EQ(expr1->getType(), typeVar1);
    EXPECT_EQ(expr2->getType(), typeVar2);

    // Solve constraints with NO arguments - demonstrates module auto-mapping
    bool success = moduleCs->solveConstraints();
    ASSERT_TRUE(success);

    // Module auto-mapping: ALL expressions in the module tree are automatically
    // updated
    EXPECT_EQ(expr1->getType(), intType); // T1 -> Int
    EXPECT_EQ(expr2->getType(), floatType); // T2 -> Float
}
