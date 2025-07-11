#include "Sema/ConstraintSystem.hpp"
#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Expr/LiteralExpr.hpp"
#include "AST/Types.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/ScopeTable.hpp"

#include <gtest/gtest.h>
#include <llvm/Support/Casting.h>

using namespace glu::sema;
using namespace glu::types;
using namespace glu::ast;

// Helper function to create a dummy locator for constraints
static ASTNode *createDummyLocator(ASTContext &context)
{
    glu::SourceLocation loc(1);
    auto *boolType = context.getTypesMemoryArena().create<BoolTy>();
    return context.getASTMemoryArena().create<LiteralExpr>(true, boolType, loc);
}

TEST(ConstraintSystem, BindToPointerType_TypeVariableToPointer)
{
    // Setup test context
    glu::SourceManager sourceManager;
    glu::DiagnosticManager diagManager(sourceManager);
    ASTContext context(&sourceManager);

    // Create a dummy module for the scope table
    glu::SourceLocation loc(1);
    auto *module = context.getASTMemoryArena().create<ModuleDecl>(
        loc, "test_module", std::vector<DeclBase *>(), &context
    );

    ScopeTable scopeTable(module);
    ConstraintSystem cs(&scopeTable, diagManager, &context);

    // Create an integer type (element type)
    auto *intType
        = context.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);

    // Create a type variable for the pointer type
    auto *pointerTypeVar
        = context.getTypesMemoryArena().create<TypeVariableTy>();
    cs.addTypeVariable(pointerTypeVar);

    // Create a BindToPointerType constraint
    // intType (element) should be first, pointerTypeVar (pointer) should be
    // second
    auto *locator = createDummyLocator(context);
    auto *constraint = Constraint::createBindToPointerType(
        cs.getAllocator(), intType, pointerTypeVar, locator
    );

    cs.addConstraint(constraint);

    // Create system state for testing
    SystemState state;
    std::vector<SystemState> worklist;

    // Apply the constraint
    bool success = cs.apply(constraint, state, worklist);

    ASSERT_TRUE(
        success
    ) << "BindToPointer constraint application should succeed";

    // Check if the type variable was bound to the correct pointer type
    auto it = state.typeBindings.find(pointerTypeVar);
    ASSERT_NE(it, state.typeBindings.end()) << "Type variable should be bound";

    auto *boundType = it->second;
    ASSERT_TRUE(llvm::isa<PointerTy>(boundType))
        << "Type variable should be bound to a pointer type";

    auto *pointerType = llvm::cast<PointerTy>(boundType);
    ASSERT_EQ(pointerType->getPointee(), intType)
        << "Pointer should point to the correct element type";
}

TEST(ConstraintSystem, BindToPointerType_ConcretePointerConsistency)
{
    // Setup test context
    glu::SourceManager sourceManager;
    glu::DiagnosticManager diagManager(sourceManager);
    ASTContext context(&sourceManager);

    // Create a dummy module for the scope table
    glu::SourceLocation loc(1);
    auto *module = context.getASTMemoryArena().create<ModuleDecl>(
        loc, "test_module", std::vector<DeclBase *>(), &context
    );

    ScopeTable scopeTable(module);
    ConstraintSystem cs(&scopeTable, diagManager, &context);

    // Create an integer type (element type)
    auto *intType
        = context.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);

    // Create a concrete pointer type
    auto *pointerType
        = context.getTypesMemoryArena().create<PointerTy>(intType);

    // Create a BindToPointerType constraint with concrete types
    auto *locator = createDummyLocator(context);
    auto *constraint = Constraint::createBindToPointerType(
        cs.getAllocator(), intType, pointerType, locator
    );

    cs.addConstraint(constraint);

    // Create system state for testing
    SystemState state;
    std::vector<SystemState> worklist;

    // Apply the constraint
    bool success = cs.apply(constraint, state, worklist);

    ASSERT_TRUE(
        success
    ) << "BindToPointer constraint with concrete types should succeed";
}

TEST(ConstraintSystem, BindToPointerType_InconsistentTypes)
{
    // Setup test context
    glu::SourceManager sourceManager;
    glu::DiagnosticManager diagManager(sourceManager);
    ASTContext context(&sourceManager);

    // Create a dummy module for the scope table
    glu::SourceLocation loc(1);
    auto *module = context.getASTMemoryArena().create<ModuleDecl>(
        loc, "test_module", std::vector<DeclBase *>(), &context
    );

    ScopeTable scopeTable(module);
    ConstraintSystem cs(&scopeTable, diagManager, &context);

    // Create two different types
    auto *intType
        = context.getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);
    auto *boolType = context.getTypesMemoryArena().create<BoolTy>();

    // Create a pointer to bool
    auto *pointerToBool
        = context.getTypesMemoryArena().create<PointerTy>(boolType);

    // Create a BindToPointerType constraint with inconsistent types
    // This should fail because we're trying to bind int as element of pointer
    // to bool
    auto *locator = createDummyLocator(context);
    auto *constraint = Constraint::createBindToPointerType(
        cs.getAllocator(), intType, pointerToBool, locator
    );

    cs.addConstraint(constraint);

    // Create system state for testing
    SystemState state;
    std::vector<SystemState> worklist;

    // Apply the constraint
    bool success = cs.apply(constraint, state, worklist);

    ASSERT_FALSE(
        success
    ) << "BindToPointer constraint with inconsistent types should fail";
}

TEST(ConstraintSystem, BindToPointerType_ElementTypeVariable)
{
    // Setup test context
    glu::SourceManager sourceManager;
    glu::DiagnosticManager diagManager(sourceManager);
    ASTContext context(&sourceManager);

    // Create a dummy module for the scope table
    glu::SourceLocation loc(1);
    auto *module = context.getASTMemoryArena().create<ModuleDecl>(
        loc, "test_module", std::vector<DeclBase *>(), &context
    );

    ScopeTable scopeTable(module);
    ConstraintSystem cs(&scopeTable, diagManager, &context);

    // Create a bool type (concrete pointee type)
    auto *boolType = context.getTypesMemoryArena().create<BoolTy>();

    // Create a concrete pointer to bool
    auto *pointerToBool
        = context.getTypesMemoryArena().create<PointerTy>(boolType);

    // Create a type variable for the element type
    auto *elementTypeVar
        = context.getTypesMemoryArena().create<TypeVariableTy>();
    cs.addTypeVariable(elementTypeVar);

    // Create a BindToPointerType constraint
    // elementTypeVar should be bound to boolType
    auto *locator = createDummyLocator(context);
    auto *constraint = Constraint::createBindToPointerType(
        cs.getAllocator(), elementTypeVar, pointerToBool, locator
    );

    cs.addConstraint(constraint);

    // Create system state for testing
    SystemState state;
    std::vector<SystemState> worklist;

    // Apply the constraint
    bool success = cs.apply(constraint, state, worklist);

    ASSERT_TRUE(success) << "BindToPointer constraint should succeed";

    // Check if the element type variable was bound correctly
    auto it = state.typeBindings.find(elementTypeVar);
    ASSERT_NE(it, state.typeBindings.end())
        << "Element type variable should be bound";

    auto *boundType = it->second;
    ASSERT_EQ(boundType, boolType)
        << "Element type variable should be bound to the correct type";
}
