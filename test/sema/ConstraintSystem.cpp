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

// Test fixture for ConstraintSystem tests to reduce setup duplication
class ConstraintSystemTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Create source manager and diagnostic manager
        sourceManager = std::make_unique<glu::SourceManager>();
        diagManager = std::make_unique<glu::DiagnosticManager>(*sourceManager);
        context = std::make_unique<ASTContext>(sourceManager.get());

        // Create a dummy module for the scope table
        glu::SourceLocation loc(1);
        module = context->getASTMemoryArena().create<ModuleDecl>(
            loc, "test_module", std::vector<DeclBase *>(), context.get()
        );

        // Create scope table and constraint system
        scopeTable = std::make_unique<ScopeTable>(module);
        cs = std::make_unique<ConstraintSystem>(
            scopeTable.get(), *diagManager, context.get()
        );
    }

    // Helper function to create a dummy locator for constraints
    ASTNode *createDummyLocator()
    {
        glu::SourceLocation loc(1);
        auto *boolType = context->getTypesMemoryArena().create<BoolTy>();
        return context->getASTMemoryArena().create<LiteralExpr>(
            true, boolType, loc
        );
    }

    // Helper function to apply a constraint and return success status
    bool applyConstraint(Constraint *constraint)
    {
        SystemState state;
        std::vector<SystemState> worklist;
        return cs->apply(constraint, state, worklist);
    }

    // Helper function to apply a constraint and get the resulting state
    std::pair<bool, SystemState>
    applyConstraintWithState(Constraint *constraint)
    {
        SystemState state;
        std::vector<SystemState> worklist;
        bool success = cs->apply(constraint, state, worklist);
        return { success, std::move(state) };
    }

    // Test fixture members
    std::unique_ptr<glu::SourceManager> sourceManager;
    std::unique_ptr<glu::DiagnosticManager> diagManager;
    std::unique_ptr<ASTContext> context;
    ModuleDecl *module;
    std::unique_ptr<ScopeTable> scopeTable;
    std::unique_ptr<ConstraintSystem> cs;
};

TEST_F(ConstraintSystemTest, BindToPointerType_TypeVariableToPointer)
{
    // Create an integer type (element type)
    auto *intType
        = context->getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);

    // Create a type variable for the pointer type
    auto *pointerTypeVar
        = context->getTypesMemoryArena().create<TypeVariableTy>();
    cs->addTypeVariable(pointerTypeVar);

    // Create a BindToPointerType constraint
    // intType (element) should be first, pointerTypeVar (pointer) should be
    // second
    auto *locator = createDummyLocator();
    auto *constraint = Constraint::createBindToPointerType(
        cs->getAllocator(), intType, pointerTypeVar, locator
    );

    cs->addConstraint(constraint);

    // Apply the constraint and get the resulting state
    auto [success, state] = applyConstraintWithState(constraint);

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

TEST_F(ConstraintSystemTest, BindToPointerType_ConcretePointerConsistency)
{
    // Create an integer type (element type)
    auto *intType
        = context->getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);

    // Create a concrete pointer type
    auto *pointerType
        = context->getTypesMemoryArena().create<PointerTy>(intType);

    // Create a BindToPointerType constraint with concrete types
    auto *locator = createDummyLocator();
    auto *constraint = Constraint::createBindToPointerType(
        cs->getAllocator(), intType, pointerType, locator
    );

    cs->addConstraint(constraint);

    // Apply the constraint
    bool success = applyConstraint(constraint);

    ASSERT_TRUE(
        success
    ) << "BindToPointer constraint with concrete types should succeed";
}

TEST_F(ConstraintSystemTest, BindToPointerType_InconsistentTypes)
{
    // Create two different types
    auto *intType
        = context->getTypesMemoryArena().create<IntTy>(IntTy::Signed, 32);
    auto *boolType = context->getTypesMemoryArena().create<BoolTy>();

    // Create a pointer to bool
    auto *pointerToBool
        = context->getTypesMemoryArena().create<PointerTy>(boolType);

    // Create a BindToPointerType constraint with inconsistent types
    // This should fail because we're trying to bind int as element of pointer
    // to bool
    auto *locator = createDummyLocator();
    auto *constraint = Constraint::createBindToPointerType(
        cs->getAllocator(), intType, pointerToBool, locator
    );

    cs->addConstraint(constraint);

    // Apply the constraint
    bool success = applyConstraint(constraint);

    ASSERT_FALSE(
        success
    ) << "BindToPointer constraint with inconsistent types should fail";
}

TEST_F(ConstraintSystemTest, BindToPointerType_ElementTypeVariable)
{
    // Create a bool type (concrete pointee type)
    auto *boolType = context->getTypesMemoryArena().create<BoolTy>();

    // Create a concrete pointer to bool
    auto *pointerToBool
        = context->getTypesMemoryArena().create<PointerTy>(boolType);

    // Create a type variable for the element type
    auto *elementTypeVar
        = context->getTypesMemoryArena().create<TypeVariableTy>();
    cs->addTypeVariable(elementTypeVar);

    // Create a BindToPointerType constraint
    // elementTypeVar should be bound to boolType
    auto *locator = createDummyLocator();
    auto *constraint = Constraint::createBindToPointerType(
        cs->getAllocator(), elementTypeVar, pointerToBool, locator
    );

    cs->addConstraint(constraint);

    // Apply the constraint and get the resulting state
    auto [success, state] = applyConstraintWithState(constraint);

    ASSERT_TRUE(success) << "BindToPointer constraint should succeed";

    // Check if the element type variable was bound correctly
    auto it = state.typeBindings.find(elementTypeVar);
    ASSERT_NE(it, state.typeBindings.end())
        << "Element type variable should be bound";

    auto *boundType = it->second;
    ASSERT_EQ(boundType, boolType)
        << "Element type variable should be bound to the correct type";
}
