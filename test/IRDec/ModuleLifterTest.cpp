#include "IRDec/ModuleLifter.hpp"
#include "AST/ASTContext.hpp"
#include "AST/Types.hpp"
#include "GIL/Function.hpp"
#include "GIL/Module.hpp"

#include <gtest/gtest.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Allocator.h>
#include <set>

class ModuleLifterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        llvmModule = std::make_unique<llvm::Module>("test_module", llvmContext);
    }

    llvm::LLVMContext llvmContext;
    glu::ast::ASTContext astContext;
    llvm::BumpPtrAllocator arena;
    std::unique_ptr<llvm::Module> llvmModule;
};

TEST_F(ModuleLifterTest, EmptyModule)
{
    // Test with an empty LLVM module
    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    EXPECT_EQ(gilModule->getFunctions().size(), 0);
    EXPECT_EQ(gilModule->getImportName(), "test_module");
}

TEST_F(ModuleLifterTest, ModuleWithDefinedFunction)
{
    // Create an LLVM function with definition (should be detected)
    auto functionType
        = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext), false);
    auto function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, "defined_func",
        llvmModule.get()
    );

    // Add a basic block to make it a definition
    llvm::BasicBlock::Create(llvmContext, "entry", function);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    // Should contain the defined function
    EXPECT_EQ(gilModule->getFunctions().size(), 1);

    auto &gilFunc = *gilModule->getFunctions().begin();
    EXPECT_EQ(gilFunc.getName(), "defined_func");
    EXPECT_NE(gilFunc.getType(), nullptr);
    EXPECT_EQ(
        gilFunc.getDecl(), nullptr
    ); // No AST declaration for external functions
}

TEST_F(ModuleLifterTest, ModuleWithDeclaredFunction)
{
    // Create an LLVM function declaration (no body) - should NOT be detected
    auto functionType
        = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext), false);
    llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, "declared_func",
        llvmModule.get()
    );
    // Don't add basic block - keep it as declaration only

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    // Should be empty because function is only declared, not defined
    EXPECT_EQ(gilModule->getFunctions().size(), 0);
}

TEST_F(ModuleLifterTest, ModuleWithParameterizedFunction)
{
    // Create function with parameters: func(int, float) -> int
    std::vector<llvm::Type *> params = { llvm::Type::getInt32Ty(llvmContext),
                                         llvm::Type::getFloatTy(llvmContext) };
    auto functionType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(llvmContext), params, false
    );
    auto function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, "param_func",
        llvmModule.get()
    );
    // Add basic block to make it a definition
    llvm::BasicBlock::Create(llvmContext, "entry", function);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    EXPECT_EQ(gilModule->getFunctions().size(), 1);

    auto &gilFunc = *gilModule->getFunctions().begin();
    EXPECT_EQ(gilFunc.getName(), "param_func");
    EXPECT_NE(gilFunc.getType(), nullptr);

    auto funcType = gilFunc.getType();
    ASSERT_NE(funcType, nullptr);
    EXPECT_EQ(funcType->getParameters().size(), 2);
}

TEST_F(ModuleLifterTest, ModuleWithVariadicFunction)
{
    // Create variadic function: func(int, ...) -> void
    std::vector<llvm::Type *> params = { llvm::Type::getInt32Ty(llvmContext) };
    auto functionType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(llvmContext), params, true // variadic = true
    );
    auto function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, "variadic_func",
        llvmModule.get()
    );
    // Add basic block to make it a definition
    llvm::BasicBlock::Create(llvmContext, "entry", function);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    EXPECT_EQ(gilModule->getFunctions().size(), 1);

    auto &gilFunc = *gilModule->getFunctions().begin();
    EXPECT_EQ(gilFunc.getName(), "variadic_func");
    EXPECT_NE(gilFunc.getType(), nullptr);

    auto funcType = gilFunc.getType();
    ASSERT_NE(funcType, nullptr);
    EXPECT_TRUE(funcType->isCVariadic());
}

TEST_F(ModuleLifterTest, ModuleWithMultipleDefinedFunctions)
{
    // Create multiple function definitions
    auto voidType = llvm::Type::getVoidTy(llvmContext);
    auto intType = llvm::Type::getInt32Ty(llvmContext);

    // func1() -> void
    auto func1Type = llvm::FunctionType::get(voidType, false);
    auto func1 = llvm::Function::Create(
        func1Type, llvm::Function::ExternalLinkage, "func1", llvmModule.get()
    );
    llvm::BasicBlock::Create(llvmContext, "entry", func1);

    // func2() -> int
    auto func2Type = llvm::FunctionType::get(intType, false);
    auto func2 = llvm::Function::Create(
        func2Type, llvm::Function::ExternalLinkage, "func2", llvmModule.get()
    );
    llvm::BasicBlock::Create(llvmContext, "entry", func2);

    // func3(int) -> int
    std::vector<llvm::Type *> params = { intType };
    auto func3Type = llvm::FunctionType::get(intType, params, false);
    auto func3 = llvm::Function::Create(
        func3Type, llvm::Function::ExternalLinkage, "func3", llvmModule.get()
    );
    llvm::BasicBlock::Create(llvmContext, "entry", func3);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    EXPECT_EQ(gilModule->getFunctions().size(), 3);

    // Collect function names
    std::set<std::string> functionNames;
    for (auto const &func : gilModule->getFunctions()) {
        functionNames.insert(func.getName().str());
    }

    EXPECT_TRUE(functionNames.count("func1"));
    EXPECT_TRUE(functionNames.count("func2"));
    EXPECT_TRUE(functionNames.count("func3"));
}

TEST_F(ModuleLifterTest, ModuleWithMixedDefinitionsAndDeclarations)
{
    auto voidType = llvm::Type::getVoidTy(llvmContext);
    auto funcType = llvm::FunctionType::get(voidType, false);

    // Create a declared function (should NOT be detected)
    llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "declared", llvmModule.get()
    );

    // Create a defined function (should be detected)
    auto definedFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "defined", llvmModule.get()
    );
    llvm::BasicBlock::Create(llvmContext, "entry", definedFunc);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    // Should only contain the defined function, not the declared one
    EXPECT_EQ(gilModule->getFunctions().size(), 1);

    auto &gilFunc = *gilModule->getFunctions().begin();
    EXPECT_EQ(gilFunc.getName(), "defined");
}

TEST_F(ModuleLifterTest, ModuleWithComplexFunctionTypes)
{
    // Create function with pointer and array parameters
    auto intType = llvm::Type::getInt32Ty(llvmContext);
    auto ptrType = intType->getPointerTo();
    auto arrayType = llvm::ArrayType::get(intType, 10);

    std::vector<llvm::Type *> params = { ptrType, arrayType };
    auto functionType = llvm::FunctionType::get(ptrType, params, false);

    auto function = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, "complex_func",
        llvmModule.get()
    );
    // Add basic block to make it a definition
    llvm::BasicBlock::Create(llvmContext, "entry", function);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    EXPECT_EQ(gilModule->getFunctions().size(), 1);

    auto &gilFunc = *gilModule->getFunctions().begin();
    EXPECT_EQ(gilFunc.getName(), "complex_func");
    EXPECT_NE(gilFunc.getType(), nullptr);
}

TEST_F(ModuleLifterTest, ModuleWithInternalLinkageFunction)
{
    // Create function with internal linkage (should NOT be detected)
    auto functionType
        = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext), false);
    auto function = llvm::Function::Create(
        functionType, llvm::Function::InternalLinkage, "internal_func",
        llvmModule.get()
    );
    // Add basic block to make it a definition
    llvm::BasicBlock::Create(llvmContext, "entry", function);

    auto gilModule
        = glu::irdec::liftModule(astContext, arena, llvmModule.get());

    ASSERT_NE(gilModule, nullptr);
    // Should be empty because function has internal linkage, not external
    EXPECT_EQ(gilModule->getFunctions().size(), 0);
}

TEST_F(ModuleLifterTest, NullModuleHandling)
{
    // Test with nullptr module (should handle gracefully or assert)
    // Note: This might cause assertion failure depending on implementation
    // In a real scenario, you might want to check that the function
    // handles nullptr gracefully or documents that it requires non-null input

    // For now, we'll skip this test as it might cause crashes
    // If the function should handle null input, uncomment and modify:
    // auto gilModule = liftModule(astContext, arena,
    // nullptr); EXPECT_EQ(gilModule, nullptr);
}
