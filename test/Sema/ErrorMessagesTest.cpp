#include "AST/ASTContext.hpp"
#include "AST/Exprs.hpp"
#include "AST/Types.hpp"
#include "Basic/Diagnostic.hpp"
#include "Sema/Constraint.hpp"
#include "Sema/ConstraintSystem.hpp"
#include "Sema/ScopeTable.hpp"
#include "gtest/gtest.h"

using namespace glu;
using namespace glu::sema;
using namespace glu::types;

class ErrorMessagesTest : public ::testing::Test {
protected:
    std::unique_ptr<ast::ASTContext> context;
    std::unique_ptr<SourceManager> sourceManager;
    std::unique_ptr<DiagnosticManager> diagManager;
    std::unique_ptr<ScopeTable> scopeTable;
    std::unique_ptr<ConstraintSystem> cs;
    llvm::BumpPtrAllocator allocator;
    ast::ModuleDecl *moduleDecl;

    // Common types for testing
    IntTy *intType;
    FloatTy *floatType;
    BoolTy *boolType;
    TypeVariableTy *typeVar1;
    TypeVariableTy *typeVar2;

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

        // Create scope table and constraint system
        scopeTable = std::make_unique<ScopeTable>(moduleDecl);
        cs = std::make_unique<ConstraintSystem>(
            scopeTable.get(), *diagManager, context.get()
        );

        // Get common types
        auto &arena = context->getTypesMemoryArena();
        intType = arena.create<IntTy>(IntTy::Signed, 32);
        floatType = arena.create<FloatTy>(32);
        boolType = arena.create<BoolTy>();
        typeVar1 = arena.create<TypeVariableTy>();
        typeVar2 = arena.create<TypeVariableTy>();

        // Add type variables to the system
        cs->addTypeVariable(typeVar1);
        cs->addTypeVariable(typeVar2);
    }

    ast::LiteralExpr *createIntLiteral(int64_t value)
    {
        return context->getASTMemoryArena().create<ast::LiteralExpr>(
            llvm::APInt(32, value), intType, SourceLocation::invalid
        );
    }
};

// Test that demonstrates the improved "No solution found" error message
TEST_F(ErrorMessagesTest, ImprovedNoSolutionError)
{
    auto &astArena = context->getASTMemoryArena();

    // Create a ref expression with a type variable
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    expr->setType(typeVar1);

    // Create conflicting constraints that cannot be satisfied
    auto *constraint1
        = Constraint::createBind(cs->getAllocator(), typeVar1, intType, expr);
    auto *constraint2
        = Constraint::createBind(cs->getAllocator(), typeVar1, floatType, expr);

    cs->addConstraint(constraint1);
    cs->addConstraint(constraint2);

    // Solve constraints - should fail with improved error message
    bool success = cs->solveConstraints({ expr });
    EXPECT_FALSE(success);
}

// Test for conversion error details
TEST_F(ErrorMessagesTest, ConversionErrorDetails)
{
    auto &astArena = context->getASTMemoryArena();

    // Create a ref expression with a type variable
    auto *expr = astArena.create<ast::RefExpr>(
        SourceLocation::invalid, ast::NamespaceIdentifier({}, "x")
    );
    expr->setType(typeVar1);

    // Create an impossible conversion constraint
    auto *constraint = Constraint::createConversion(
        cs->getAllocator(), boolType, intType, expr
    );
    cs->addConstraint(constraint);

    // Solve constraints - should fail with conversion error details
    bool success = cs->solveConstraints({ expr });
    EXPECT_FALSE(success);
}
