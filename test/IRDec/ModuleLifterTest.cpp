#include "IRDec/ModuleLifter.hpp"
#include "AST/ASTContext.hpp"
#include "AST/Decls.hpp"
#include "AST/Types.hpp"

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <gtest/gtest.h>
#include <llvm/Support/Casting.h>

using namespace glu;

class ModuleLifterTest : public ::testing::Test {
protected:
    llvm::LLVMContext ctx;
    std::unique_ptr<llvm::Module> module;
    glu::ast::ASTContext astCtx;

    ModuleLifterTest() : astCtx()
    {
        module = std::make_unique<llvm::Module>("test_module", ctx);
    }

    void SetUp() override { }
};

TEST_F(ModuleLifterTest, LiftEmptyModule)
{
    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 0);
}

// Suppress unused variable warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

TEST_F(ModuleLifterTest, LiftModuleWithSingleExternalFunction)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    // Create debug info for function: i32 add(i32, i32)
    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    llvm::SmallVector<llvm::Metadata *, 3> paramTypes;
    paramTypes.push_back(i32Type); // return type
    paramTypes.push_back(i32Type); // param 0
    paramTypes.push_back(i32Type); // param 1

    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    // Create the function with external linkage
    auto *funcTy = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ctx),
        { llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx) }, false
    );
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "add", module.get()
    );

    // Add debug info to the function
    auto *sp = dib.createFunction(
        cu, "add", "add", file, 1, funcType, 1, llvm::DINode::FlagPrototyped,
        llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);

    // Create a basic block to make it not a declaration
    llvm::BasicBlock::Create(ctx, "entry", func);

    dib.finalize();

    // Lift the module
    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1);

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "add");

    auto *funcDeclType = funcDecl->getType();
    ASSERT_NE(funcDeclType, nullptr);
    auto *functionTy = llvm::dyn_cast<types::FunctionTy>(funcDeclType);
    ASSERT_NE(functionTy, nullptr);

    // Verify return type
    auto *returnType = functionTy->getReturnType();
    ASSERT_NE(returnType, nullptr);
    auto *returnIntTy = llvm::dyn_cast<types::IntTy>(returnType);
    ASSERT_NE(returnIntTy, nullptr);
    EXPECT_EQ(returnIntTy->getBitWidth(), 32);
    EXPECT_EQ(returnIntTy->getSignedness(), types::IntTy::Signed);

    // Verify parameters
    EXPECT_EQ(functionTy->getParameterCount(), 2);
    EXPECT_EQ(funcDecl->getParams().size(), 2);

    auto *param0Type = functionTy->getParameter(0);
    ASSERT_NE(param0Type, nullptr);
    auto *param0IntTy = llvm::dyn_cast<types::IntTy>(param0Type);
    ASSERT_NE(param0IntTy, nullptr);
    EXPECT_EQ(param0IntTy->getBitWidth(), 32);

    auto *param1Type = functionTy->getParameter(1);
    ASSERT_NE(param1Type, nullptr);
    auto *param1IntTy = llvm::dyn_cast<types::IntTy>(param1Type);
    ASSERT_NE(param1IntTy, nullptr);
    EXPECT_EQ(param1IntTy->getBitWidth(), 32);

    // Verify parameter names
    EXPECT_EQ(funcDecl->getParams()[0]->getName(), "param0");
    EXPECT_EQ(funcDecl->getParams()[1]->getName(), "param1");

    // Verify function has no body
    EXPECT_EQ(funcDecl->getBody(), nullptr);
}

TEST_F(ModuleLifterTest, LiftModuleWithMultipleExternalFunctions)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    auto *f32Type = dib.createBasicType("f32", 32, llvm::dwarf::DW_ATE_float);
    auto *voidType
        = dib.createBasicType("void", 0, llvm::dwarf::DW_ATE_address);

    // Function 1: i32 add(i32, i32)
    {
        llvm::SmallVector<llvm::Metadata *, 3> paramTypes;
        paramTypes.push_back(i32Type);
        paramTypes.push_back(i32Type);
        paramTypes.push_back(i32Type);
        auto *funcType
            = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

        auto *funcTy = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ctx),
            { llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx) }, false
        );
        auto *func = llvm::Function::Create(
            funcTy, llvm::Function::ExternalLinkage, "add", module.get()
        );
        auto *sp = dib.createFunction(
            cu, "add", "add", file, 1, funcType, 1,
            llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
        );
        func->setSubprogram(sp);
        llvm::BasicBlock::Create(ctx, "entry", func);
    }

    // Function 2: f32 sqrt(f32)
    {
        llvm::SmallVector<llvm::Metadata *, 2> paramTypes;
        paramTypes.push_back(f32Type);
        paramTypes.push_back(f32Type);
        auto *funcType
            = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

        auto *funcTy = llvm::FunctionType::get(
            llvm::Type::getFloatTy(ctx), { llvm::Type::getFloatTy(ctx) }, false
        );
        llvm::Function::Create(
            funcTy, llvm::Function::ExternalLinkage, "sqrt", module.get()
        );
        auto *sp = dib.createFunction(
            cu, "sqrt", "sqrt", file, 5, funcType, 5,
            llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
        );

        // Need to get the function back to set subprogram
        auto *sqrtFunc = module->getFunction("sqrt");
        sqrtFunc->setSubprogram(sp);
        llvm::BasicBlock::Create(ctx, "entry", sqrtFunc);
    }

    // Function 3: void print(i32)
    {
        llvm::SmallVector<llvm::Metadata *, 2> paramTypes;
        paramTypes.push_back(voidType);
        paramTypes.push_back(i32Type);
        auto *funcType
            = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

        auto *funcTy = llvm::FunctionType::get(
            llvm::Type::getVoidTy(ctx), { llvm::Type::getInt32Ty(ctx) }, false
        );
        llvm::Function::Create(
            funcTy, llvm::Function::ExternalLinkage, "print", module.get()
        );
        auto *sp = dib.createFunction(
            cu, "print", "print", file, 10, funcType, 10,
            llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
        );

        // Need to get the function back to set subprogram
        auto *printFunc = module->getFunction("print");
        printFunc->setSubprogram(sp);
        llvm::BasicBlock::Create(ctx, "entry", printFunc);
    }

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 3);

    // Verify all three functions are present
    bool foundAdd = false, foundSqrt = false, foundPrint = false;
    for (auto *decl : moduleDecl->getDecls()) {
        auto *funcDecl = llvm::dyn_cast<ast::FunctionDecl>(decl);
        ASSERT_NE(funcDecl, nullptr);

        if (funcDecl->getName() == "add") {
            foundAdd = true;
            EXPECT_EQ(funcDecl->getParams().size(), 2);
        } else if (funcDecl->getName() == "sqrt") {
            foundSqrt = true;
            EXPECT_EQ(funcDecl->getParams().size(), 1);
        } else if (funcDecl->getName() == "print") {
            foundPrint = true;
            EXPECT_EQ(funcDecl->getParams().size(), 1);
        }
    }

    EXPECT_TRUE(foundAdd);
    EXPECT_TRUE(foundSqrt);
    EXPECT_TRUE(foundPrint);
}

TEST_F(ModuleLifterTest, LiftModuleWithStructType)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    auto *f32Type = dib.createBasicType("f32", 32, llvm::dwarf::DW_ATE_float);

    // Create a struct: Point { x: i32, y: f32 }
    llvm::SmallVector<llvm::Metadata *, 2> elements;
    elements.push_back(dib.createMemberType(
        cu, "x", file, 1, 32, 32, 0, llvm::DINode::FlagZero, i32Type
    ));
    elements.push_back(dib.createMemberType(
        cu, "y", file, 2, 32, 32, 32, llvm::DINode::FlagZero, f32Type
    ));

    auto *structType = dib.createStructType(
        cu, "Point", file, 1, 64, 32, llvm::DINode::FlagZero, nullptr,
        dib.getOrCreateArray(elements)
    );

    // Create a function that returns the struct: Point makePoint()
    llvm::SmallVector<llvm::Metadata *, 1> paramTypes;
    paramTypes.push_back(structType);
    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    auto *structTy = llvm::StructType::create(
        ctx, { llvm::Type::getInt32Ty(ctx), llvm::Type::getFloatTy(ctx) },
        "Point"
    );
    auto *funcTy = llvm::FunctionType::get(structTy, {}, false);
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "makePoint", module.get()
    );
    auto *sp = dib.createFunction(
        cu, "makePoint", "makePoint", file, 5, funcType, 5,
        llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);
    llvm::BasicBlock::Create(ctx, "entry", func);

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 2); // function + struct

    // Find the function and struct declarations
    ast::FunctionDecl *funcDecl = nullptr;
    ast::StructDecl *structDecl = nullptr;

    for (auto *decl : moduleDecl->getDecls()) {
        if (auto *fd = llvm::dyn_cast<ast::FunctionDecl>(decl)) {
            funcDecl = fd;
        } else if (auto *sd = llvm::dyn_cast<ast::StructDecl>(decl)) {
            structDecl = sd;
        }
    }

    ASSERT_NE(funcDecl, nullptr);
    ASSERT_NE(structDecl, nullptr);

    // Verify struct
    EXPECT_EQ(structDecl->getName(), "Point");
    EXPECT_EQ(structDecl->getFields().size(), 2);
    EXPECT_EQ(structDecl->getFields()[0]->getName(), "x");
    EXPECT_EQ(structDecl->getFields()[1]->getName(), "y");

    // Verify field types
    auto *xType = structDecl->getFields()[0]->getType();
    ASSERT_NE(xType, nullptr);
    auto *xIntTy = llvm::dyn_cast<types::IntTy>(xType);
    ASSERT_NE(xIntTy, nullptr);
    EXPECT_EQ(xIntTy->getBitWidth(), 32);

    auto *yType = structDecl->getFields()[1]->getType();
    ASSERT_NE(yType, nullptr);
    auto *yFloatTy = llvm::dyn_cast<types::FloatTy>(yType);
    ASSERT_NE(yFloatTy, nullptr);
    EXPECT_EQ(yFloatTy->getBitWidth(), 32);

    // Verify function return type is the struct
    auto *returnType = funcDecl->getType()->getReturnType();
    ASSERT_NE(returnType, nullptr);
    auto *returnStructTy = llvm::dyn_cast<types::StructTy>(returnType);
    ASSERT_NE(returnStructTy, nullptr);
    EXPECT_EQ(returnStructTy->getDecl(), structDecl);
}

TEST_F(ModuleLifterTest, LiftModuleWithEnumType)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);

    // Create an enum: Color { Red, Green, Blue }
    llvm::SmallVector<llvm::Metadata *, 3> enumerators;
    enumerators.push_back(dib.createEnumerator("Red", 0));
    enumerators.push_back(dib.createEnumerator("Green", 1));
    enumerators.push_back(dib.createEnumerator("Blue", 2));

    auto *enumType = dib.createEnumerationType(
        cu, "Color", file, 1, 32, 32, dib.getOrCreateArray(enumerators), i32Type
    );

    // Create a function that uses the enum: Color getColor()
    llvm::SmallVector<llvm::Metadata *, 1> paramTypes;
    paramTypes.push_back(enumType);
    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    auto *funcTy
        = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false);
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "getColor", module.get()
    );
    auto *sp = dib.createFunction(
        cu, "getColor", "getColor", file, 5, funcType, 5,
        llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);
    llvm::BasicBlock::Create(ctx, "entry", func);

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 2); // function + enum

    // Find the function and enum declarations
    ast::FunctionDecl *funcDecl = nullptr;
    ast::EnumDecl *enumDecl = nullptr;

    for (auto *decl : moduleDecl->getDecls()) {
        if (auto *fd = llvm::dyn_cast<ast::FunctionDecl>(decl)) {
            funcDecl = fd;
        } else if (auto *ed = llvm::dyn_cast<ast::EnumDecl>(decl)) {
            enumDecl = ed;
        }
    }

    ASSERT_NE(funcDecl, nullptr);
    ASSERT_NE(enumDecl, nullptr);

    // Verify enum
    EXPECT_EQ(enumDecl->getName(), "Color");
    EXPECT_EQ(enumDecl->getFields().size(), 3);
    EXPECT_EQ(enumDecl->getFields()[0]->getName(), "Red");
    EXPECT_EQ(enumDecl->getFields()[1]->getName(), "Green");
    EXPECT_EQ(enumDecl->getFields()[2]->getName(), "Blue");

    // Verify function return type is the enum
    auto *returnType = funcDecl->getType()->getReturnType();
    ASSERT_NE(returnType, nullptr);
    auto *returnEnumTy = llvm::dyn_cast<types::EnumTy>(returnType);
    ASSERT_NE(returnEnumTy, nullptr);
    EXPECT_EQ(returnEnumTy->getDecl(), enumDecl);
}

TEST_F(ModuleLifterTest, LiftModuleWithComplexTypes)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);

    // Create a pointer type
    auto *ptrType = dib.createPointerType(i32Type, 64);

    // Create a function: i32* allocate()
    llvm::SmallVector<llvm::Metadata *, 1> paramTypes;
    paramTypes.push_back(ptrType);
    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    auto *funcTy = llvm::FunctionType::get(
        llvm::PointerType::get(llvm::Type::getInt32Ty(ctx), 0), {}, false
    );
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "allocate", module.get()
    );
    auto *sp = dib.createFunction(
        cu, "allocate", "allocate", file, 1, funcType, 1,
        llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);
    llvm::BasicBlock::Create(ctx, "entry", func);

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1);

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "allocate");

    // Verify return type is a pointer to i32
    auto *returnType = funcDecl->getType()->getReturnType();
    ASSERT_NE(returnType, nullptr);
    auto *ptrTy = llvm::dyn_cast<types::PointerTy>(returnType);
    ASSERT_NE(ptrTy, nullptr);

    auto *pointeeType = ptrTy->getPointee();
    ASSERT_NE(pointeeType, nullptr);
    auto *intTy = llvm::dyn_cast<types::IntTy>(pointeeType);
    ASSERT_NE(intTy, nullptr);
    EXPECT_EQ(intTy->getBitWidth(), 32);
}

TEST_F(ModuleLifterTest, IgnoreInternalLinkageFunctions)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);

    // External linkage function
    {
        llvm::SmallVector<llvm::Metadata *, 1> paramTypes;
        paramTypes.push_back(i32Type);
        auto *funcType
            = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

        auto *funcTy
            = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false);
        auto *func = llvm::Function::Create(
            funcTy, llvm::Function::ExternalLinkage, "publicFunc", module.get()
        );
        auto *sp = dib.createFunction(
            cu, "publicFunc", "publicFunc", file, 1, funcType, 1,
            llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
        );
        func->setSubprogram(sp);
        llvm::BasicBlock::Create(ctx, "entry", func);
    }

    // Internal linkage function (should be ignored)
    {
        llvm::SmallVector<llvm::Metadata *, 1> paramTypes;
        paramTypes.push_back(i32Type);
        auto *funcType
            = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

        auto *funcTy
            = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false);
        auto *func = llvm::Function::Create(
            funcTy, llvm::Function::InternalLinkage, "privateFunc", module.get()
        );
        auto *sp = dib.createFunction(
            cu, "privateFunc", "privateFunc", file, 5, funcType, 5,
            llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
        );
        func->setSubprogram(sp);
        llvm::BasicBlock::Create(ctx, "entry", func);
    }

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1); // Only the external function

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "publicFunc");
}

TEST_F(ModuleLifterTest, IgnoreFunctionDeclarations)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    dib.createCompileUnit(llvm::dwarf::DW_LANG_C, file, "test", false, "", 0);

    // Create a function declaration (no body)
    auto *funcTy
        = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false);
    llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "declaredFunc", module.get()
    );

    dib.finalize();

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(
        moduleDecl->getDecls().size(), 0
    ); // Function declarations are ignored
}

TEST_F(ModuleLifterTest, LiftFunctionWithoutDebugInfo)
{
    // Create a function with external linkage but no debug info
    auto *funcTy
        = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false);
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "noDebugInfo", module.get()
    );
    llvm::BasicBlock::Create(ctx, "entry", func);

    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1);

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "noDebugInfo");

    // Verify the function type was lifted correctly
    auto *funcDeclType = funcDecl->getType();
    ASSERT_NE(funcDeclType, nullptr);
    auto *functionTy = llvm::dyn_cast<types::FunctionTy>(funcDeclType);
    ASSERT_NE(functionTy, nullptr);

    // Verify return type is i32
    auto *returnType = functionTy->getReturnType();
    ASSERT_NE(returnType, nullptr);
    auto *returnIntTy = llvm::dyn_cast<types::IntTy>(returnType);
    ASSERT_NE(returnIntTy, nullptr);
    EXPECT_EQ(returnIntTy->getBitWidth(), 32);

    // Verify no parameters
    EXPECT_EQ(functionTy->getParameterCount(), 0);
    EXPECT_EQ(funcDecl->getParams().size(), 0);
}

TEST_F(ModuleLifterTest, LiftFunctionWithParameterNamesFromDebugInfo)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    // Create debug info for function: i32 multiply(i32 lhs, i32 rhs)
    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    llvm::SmallVector<llvm::Metadata *, 3> paramTypes;
    paramTypes.push_back(i32Type); // return type
    paramTypes.push_back(i32Type); // param 0
    paramTypes.push_back(i32Type); // param 1

    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    // Create the function with external linkage
    auto *funcTy = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ctx),
        { llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx) }, false
    );
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "multiply", module.get()
    );

    // Add debug info to the function
    auto *sp = dib.createFunction(
        cu, "multiply", "multiply", file, 1, funcType, 1,
        llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);

    // Create a basic block and add debug variable declarations
    auto *bb = llvm::BasicBlock::Create(ctx, "entry", func);
    llvm::IRBuilder<> builder(bb);

    // Create debug variables for parameters with argument indices
    auto *lhsVar = dib.createParameterVariable(
        sp, "lhs", 1, file, 1, i32Type, true, llvm::DINode::FlagZero
    );
    auto *rhsVar = dib.createParameterVariable(
        sp, "rhs", 2, file, 1, i32Type, true, llvm::DINode::FlagZero
    );

    // Insert dbg.declare intrinsics using builder
    auto *lhsDbgLoc = llvm::DILocation::get(ctx, 1, 0, sp);
    builder.SetCurrentDebugLocation(lhsDbgLoc);
    dib.insertDeclare(
        func->getArg(0), lhsVar, dib.createExpression(), lhsDbgLoc,
        builder.GetInsertBlock()
    );

    auto *rhsDbgLoc = llvm::DILocation::get(ctx, 1, 0, sp);
    builder.SetCurrentDebugLocation(rhsDbgLoc);
    dib.insertDeclare(
        func->getArg(1), rhsVar, dib.createExpression(), rhsDbgLoc,
        builder.GetInsertBlock()
    );

    // Add a return to terminate the block
    builder.CreateRet(builder.getInt32(0));

    dib.finalize();

    // Lift the module
    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1);

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "multiply");

    // Verify parameters have names from debug info
    EXPECT_EQ(funcDecl->getParams().size(), 2);
    EXPECT_EQ(funcDecl->getParams()[0]->getName(), "lhs");
    EXPECT_EQ(funcDecl->getParams()[1]->getName(), "rhs");
}

TEST_F(ModuleLifterTest, LiftFunctionWithPartialParameterNamesFromDebugInfo)
{
    llvm::DIBuilder dib(*module);
    auto *file = dib.createFile("test.glu", ".");
    auto *cu = dib.createCompileUnit(
        llvm::dwarf::DW_LANG_C, file, "test", false, "", 0
    );

    // Create debug info for function: i32 compute(i32, i32, i32)
    auto *i32Type = dib.createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    llvm::SmallVector<llvm::Metadata *, 4> paramTypes;
    paramTypes.push_back(i32Type); // return type
    paramTypes.push_back(i32Type); // param 0
    paramTypes.push_back(i32Type); // param 1
    paramTypes.push_back(i32Type); // param 2

    auto *funcType
        = dib.createSubroutineType(dib.getOrCreateTypeArray(paramTypes));

    // Create the function with external linkage
    auto *funcTy = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ctx),
        { llvm::Type::getInt32Ty(ctx), llvm::Type::getInt32Ty(ctx),
          llvm::Type::getInt32Ty(ctx) },
        false
    );
    auto *func = llvm::Function::Create(
        funcTy, llvm::Function::ExternalLinkage, "compute", module.get()
    );

    // Add debug info to the function
    auto *sp = dib.createFunction(
        cu, "compute", "compute", file, 1, funcType, 1,
        llvm::DINode::FlagPrototyped, llvm::DISubprogram::SPFlagDefinition
    );
    func->setSubprogram(sp);

    // Create a basic block and add debug variable declarations only for first
    // and third parameters
    auto *bb = llvm::BasicBlock::Create(ctx, "entry", func);
    llvm::IRBuilder<> builder(bb);

    // Create debug variables for only first and third parameters
    auto *xVar = dib.createParameterVariable(
        sp, "x", 1, file, 1, i32Type, true, llvm::DINode::FlagZero
    );
    auto *zVar = dib.createParameterVariable(
        sp, "z", 3, file, 1, i32Type, true, llvm::DINode::FlagZero
    );

    // Insert dbg.declare intrinsics using builder
    auto *xDbgLoc = llvm::DILocation::get(ctx, 1, 0, sp);
    builder.SetCurrentDebugLocation(xDbgLoc);
    dib.insertDeclare(
        func->getArg(0), xVar, dib.createExpression(), xDbgLoc,
        builder.GetInsertBlock()
    );

    auto *zDbgLoc = llvm::DILocation::get(ctx, 1, 0, sp);
    builder.SetCurrentDebugLocation(zDbgLoc);
    dib.insertDeclare(
        func->getArg(2), zVar, dib.createExpression(), zDbgLoc,
        builder.GetInsertBlock()
    );

    // Add a return to terminate the block
    builder.CreateRet(builder.getInt32(0));

    dib.finalize();

    // Lift the module
    auto *moduleDecl = irdec::liftModule(astCtx, module.get());

    ASSERT_NE(moduleDecl, nullptr);
    EXPECT_EQ(moduleDecl->getDecls().size(), 1);

    auto *funcDecl
        = llvm::dyn_cast<ast::FunctionDecl>(moduleDecl->getDecls()[0]);
    ASSERT_NE(funcDecl, nullptr);
    EXPECT_EQ(funcDecl->getName(), "compute");

    // Verify parameters: first has name "x", second has default "param1", third
    // has name "z"
    EXPECT_EQ(funcDecl->getParams().size(), 3);
    EXPECT_EQ(funcDecl->getParams()[0]->getName(), "x");
    EXPECT_EQ(funcDecl->getParams()[1]->getName(), "param1");
    EXPECT_EQ(funcDecl->getParams()[2]->getName(), "z");
}

#pragma GCC diagnostic pop
