#include "AST/ASTContext.hpp"
#include "AST/Types.hpp"

#include "GIL/Function.hpp"
#include "GIL/Instructions.hpp"
#include "GIL/Module.hpp"
#include "GIL/Type.hpp"

#include "IRGen/IRGen.hpp"

#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Allocator.h>

#include <gtest/gtest.h>

class IRGenTest : public ::testing::Test {
protected:
    llvm::LLVMContext ctx;
    llvm::Module llvmModule;
    llvm::BumpPtrAllocator allocator;
    glu::ast::ASTContext astCtx;
    glu::gil::Module gilModule;
    glu::types::IntTy *intTy;
    glu::gil::Type gilIntTy;
    glu::gil::Type gilPtrTy;

    IRGenTest()
        : llvmModule("test", ctx)
        , gilModule("test")
        , intTy(astCtx.getTypesMemoryArena().create<glu::types::IntTy>(
              glu::types::IntTy::Signed, 32
          ))
        , gilIntTy(4, 4, false, intTy)
        , gilPtrTy(
              8, 8, false,
              astCtx.getTypesMemoryArena().create<glu::types::PointerTy>(intTy)
          )
    {
        // gilFunc and funcTy creation moved to test body
    }

    glu::gil::BasicBlock *createEntry(glu::gil::Function *func)
    {
        auto *entry = glu::gil::BasicBlock::create(allocator, "entry", {});
        func->getBasicBlocks().push_back(entry);
        return entry;
    }
};

TEST_F(IRGenTest, AllocaStoreLoad_GeneratesAllocaStoreLoad)
{
    auto *funcTy = astCtx.getTypesMemoryArena().create<glu::types::FunctionTy>(
        std::vector<glu::types::TypeBase *> {}, intTy
    );
    glu::gil::Function *gilFunc = gilModule.addFunction("testFunc", funcTy);
    auto *entry = createEntry(gilFunc);
    // Allocate memory
    auto *allocaInst = new (allocator) glu::gil::AllocaInst(gilIntTy, gilPtrTy);
    entry->getInstructions().push_back(allocaInst);
    // Store value into allocated memory
    llvm::APInt value(32, 42);
    auto *intLitInst
        = glu::gil::IntegerLiteralInst::create(allocator, gilIntTy, value);
    entry->getInstructions().push_back(intLitInst);
    auto *storeInst = new (allocator)
        glu::gil::StoreInst(intLitInst->getResult(0), allocaInst->getResult(0));
    entry->getInstructions().push_back(storeInst);
    // Load value from allocated memory
    auto *loadInst = new (allocator)
        glu::gil::LoadInst(allocaInst->getResult(0), gilIntTy);
    entry->getInstructions().push_back(loadInst);
    // Return the integer literal
    auto *retInst
        = new (allocator) glu::gil::ReturnInst(loadInst->getResult(0));
    entry->getInstructions().push_back(retInst);
    // Generate IR and check for alloca, store, and load
    glu::irgen::IRGen irgen;
    irgen.generateIR(llvmModule, &gilModule);
    // Assert there is a single function, a single basic block, and exactly 4
    // instructions
    ASSERT_EQ(std::distance(llvmModule.begin(), llvmModule.end()), 1);
    auto &func = *llvmModule.begin();
    ASSERT_EQ(std::distance(func.begin(), func.end()), 1);
    auto &bb = *func.begin();
    ASSERT_EQ(std::distance(bb.begin(), bb.end()), 4);
    // Check that each instruction is of the expected type
    auto it = bb.begin();
    ASSERT_TRUE(llvm::isa<llvm::AllocaInst>(&*it));
    ++it;
    ASSERT_TRUE(llvm::isa<llvm::StoreInst>(&*it));
    ++it;
    ASSERT_TRUE(llvm::isa<llvm::LoadInst>(&*it));
    ++it;
    ASSERT_TRUE(llvm::isa<llvm::ReturnInst>(&*it));

    // llvmModule.print(llvm::outs(), nullptr);
}

TEST_F(IRGenTest, EnumReturn_GeneratesEnumConstantReturn)
{
    // Create enum type and variant
    std::vector<glu::types::Case> cases = { { "A", llvm::APInt(32, 0) },
                                            { "B", llvm::APInt(32, 1) },
                                            { "C", llvm::APInt(32, 2) },
                                            { "D", llvm::APInt(32, 3) } };
    auto *enumTy = glu::types::EnumTy::create(
        allocator, "MyEnum", cases, glu::SourceLocation(0)
    );
    glu::gil::Type gilEnumTy(4, 4, false, enumTy);
    // Re-create function with enum return type
    auto *enumFuncTy
        = astCtx.getTypesMemoryArena().create<glu::types::FunctionTy>(
            std::vector<glu::types::TypeBase *> {}, enumTy
        );
    glu::gil::Function *enumFunc
        = gilModule.addFunction("enumFunc", enumFuncTy);
    auto *entry = glu::gil::BasicBlock::create(allocator, "entry", {});
    enumFunc->getBasicBlocks().push_back(entry);
    // Create enum variant instruction
    glu::gil::Member member("C", gilEnumTy, gilEnumTy);
    auto *enumInst = new (allocator) glu::gil::EnumVariantInst(member);
    entry->getInstructions().push_back(enumInst);
    // Return the enum constant
    auto *gilRetInst
        = new (allocator) glu::gil::ReturnInst(enumInst->getResult(0));
    entry->getInstructions().push_back(gilRetInst);
    // Generate IR and check for return of enum constant
    glu::irgen::IRGen irgen;
    irgen.generateIR(llvmModule, &gilModule);
    // Assert there is a single function, a single basic block, and exactly 1
    // instruction
    ASSERT_EQ(
        std::distance(llvmModule.begin(), llvmModule.end()), 1
    ); // Only one function
    auto &func = *llvmModule.begin();
    ASSERT_EQ(std::distance(func.begin(), func.end()), 1);
    auto &bb = *func.begin();
    ASSERT_EQ(std::distance(bb.begin(), bb.end()), 1);
    // Check that the instruction is a return
    auto it = bb.begin();
    ASSERT_TRUE(llvm::isa<llvm::ReturnInst>(&*it));
    // Check that the return value is a constant (enum value)
    auto *llvmRetInst = llvm::cast<llvm::ReturnInst>(&*it);
    auto *retVal = llvmRetInst->getReturnValue();
    ASSERT_TRUE(retVal != nullptr);
    ASSERT_TRUE(llvm::isa<llvm::ConstantInt>(retVal));
    auto *constVal = llvm::cast<llvm::ConstantInt>(retVal);
    ASSERT_EQ(constVal->getValue().getZExtValue(), 2); // 'C' variant value

    // llvmModule.print(llvm::outs(), nullptr);
}

// Add more tests for other instructions as needed
