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
    glu::gil::Module &gilModule;
    glu::types::IntTy *intTy;
    glu::gil::Type gilIntTy;
    glu::types::BoolTy *boolTy;
    glu::gil::Type gilBoolTy;
    glu::gil::Type gilPtrTy;

    IRGenTest()
        : llvmModule("test", ctx)
        , gilModule(*new(allocator) glu::gil::Module("test"))
        , intTy(astCtx.getTypesMemoryArena().create<glu::types::IntTy>(
              glu::types::IntTy::Signed, 32
          ))
        , gilIntTy(4, 4, false, intTy)
        , boolTy(astCtx.getTypesMemoryArena().create<glu::types::BoolTy>())
        , gilBoolTy(1, 1, false, boolTy)
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
    glu::gil::Function *gilFunc
        = new (allocator) glu::gil::Function("testFunc", funcTy, nullptr);
    gilModule.addFunction(gilFunc);
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
    auto *loadInst = new (allocator) glu::gil::LoadInst(
        allocaInst->getResult(0), gilIntTy, glu::gil::LoadOwnershipKind::None
    );
    entry->getInstructions().push_back(loadInst);
    // Return the integer literal
    auto *retInst
        = new (allocator) glu::gil::ReturnInst(loadInst->getResult(0));
    entry->getInstructions().push_back(retInst);
    // Generate IR and check for alloca, store, and load
    glu::irgen::IRGen irgen;
    irgen.generateIR(llvmModule, &gilModule, nullptr);
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
}

TEST_F(IRGenTest, EnumReturn_GeneratesEnumConstantReturn)
{
    llvm::SmallVector<glu::ast::FieldDecl *> fields {
        new (allocator)
            glu::ast::FieldDecl(glu::SourceLocation(0), "A", nullptr, nullptr),
        new (allocator)
            glu::ast::FieldDecl(glu::SourceLocation(0), "B", nullptr, nullptr),
        new (allocator)
            glu::ast::FieldDecl(glu::SourceLocation(0), "C", nullptr, nullptr),
        new (allocator)
            glu::ast::FieldDecl(glu::SourceLocation(0), "D", nullptr, nullptr),
    };

    auto *enumDecl = glu::ast::EnumDecl::create(
        allocator, astCtx, glu::SourceLocation(0), nullptr, "TestEnum", fields
    );
    auto *enumTy = enumDecl->getType();

    glu::gil::Type gilEnumTy(4, 4, false, enumTy);
    // Re-create function with enum return type
    auto *enumFuncTy
        = astCtx.getTypesMemoryArena().create<glu::types::FunctionTy>(
            std::vector<glu::types::TypeBase *> {}, enumTy
        );
    glu::gil::Function *enumFunc
        = new (allocator) glu::gil::Function("enumFunc", enumFuncTy, nullptr);
    gilModule.addFunction(enumFunc);
    auto *entry = createEntry(enumFunc);
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
    irgen.generateIR(llvmModule, &gilModule, nullptr);
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
}

TEST_F(IRGenTest, PhiNode_MultiplePredecessors_GeneratesCorrectPhiNode)
{
    // Function type: int(bool)
    auto *funcTy = astCtx.getTypesMemoryArena().create<glu::types::FunctionTy>(
        std::vector<glu::types::TypeBase *> { boolTy }, intTy
    );
    glu::gil::Function *gilFunc = new (allocator)
        glu::gil::Function("phiFuncMultiPred", funcTy, nullptr);
    gilModule.addFunction(gilFunc);

    // Entry block with one argument (x: bool)
    auto *entry
        = glu::gil::BasicBlock::create(allocator, "entry", { gilBoolTy });
    gilFunc->getBasicBlocks().push_back(entry);

    // Create then and else blocks
    auto *thenBlock = glu::gil::BasicBlock::create(allocator, "then", {});
    auto *elseBlock = glu::gil::BasicBlock::create(allocator, "else", {});
    // Merge block with one argument
    auto *mergeBlock
        = glu::gil::BasicBlock::create(allocator, "merge", { gilIntTy });
    gilFunc->getBasicBlocks().push_back(thenBlock);
    gilFunc->getBasicBlocks().push_back(elseBlock);
    gilFunc->getBasicBlocks().push_back(mergeBlock);

    // Integer literal 1 and 2
    auto *oneInst = glu::gil::IntegerLiteralInst::create(
        allocator, gilIntTy, llvm::APInt(32, 1)
    );
    auto *twoInst = glu::gil::IntegerLiteralInst::create(
        allocator, gilIntTy, llvm::APInt(32, 2)
    );
    entry->getInstructions().push_back(oneInst);
    entry->getInstructions().push_back(twoInst);

    // Use entry argument as the condition (simulate x != 0)
    auto condValue = entry->getArgument(0);
    auto *condBr = glu::gil::CondBrInst::create(
        allocator, condValue, thenBlock, elseBlock, {}, {}
    );
    entry->getInstructions().push_back(condBr);

    // In thenBlock, branch to mergeBlock with argument 1
    auto *brThenToMerge = glu::gil::BrInst::create(
        allocator, mergeBlock, { oneInst->getResult(0) }
    );
    thenBlock->getInstructions().push_back(brThenToMerge);
    // In elseBlock, branch to mergeBlock with argument 2
    auto *brElseToMerge = glu::gil::BrInst::create(
        allocator, mergeBlock, { twoInst->getResult(0) }
    );
    elseBlock->getInstructions().push_back(brElseToMerge);

    // In mergeBlock, return the block argument (should be a phi node)
    auto mergeArg = mergeBlock->getArgument(0);
    auto *retInst = new (allocator) glu::gil::ReturnInst(mergeArg);
    mergeBlock->getInstructions().push_back(retInst);

    // Generate IR
    glu::irgen::IRGen irgen;
    irgen.generateIR(llvmModule, &gilModule, nullptr);

    // Assert function, blocks, and phi node
    ASSERT_EQ(
        std::distance(llvmModule.begin(), llvmModule.end()), 1
    ); // Only one function
    auto &func = *llvmModule.begin();
    ASSERT_EQ(
        std::distance(func.begin(), func.end()), 4
    ); // entry, then, else, merge
    // Find merge block
    llvm::BasicBlock *mergeBB = nullptr;
    for (auto &block : func) {
        if (block.getName() == "merge") {
            mergeBB = &block;
            break;
        }
    }
    ASSERT_TRUE(mergeBB != nullptr);
    // Find phi node in merge block
    auto it = mergeBB->begin();
    ASSERT_TRUE(llvm::isa<llvm::PHINode>(&*it));
    auto *phi = llvm::cast<llvm::PHINode>(&*it);
    ASSERT_EQ(phi->getNumIncomingValues(), 2); // Two incoming edges
    // Check incoming values and their blocks
    std::vector<uint64_t> incomingVals;
    std::vector<std::string> incomingBlocks;
    for (unsigned i = 0; i < phi->getNumIncomingValues(); ++i) {
        auto *incoming = phi->getIncomingValue(i);
        ASSERT_TRUE(llvm::isa<llvm::ConstantInt>(incoming));
        incomingVals.push_back(
            llvm::cast<llvm::ConstantInt>(incoming)->getValue().getZExtValue()
        );
        incomingBlocks.push_back(phi->getIncomingBlock(i)->getName().str());
    }
    // Should have values 1 and 2, and blocks 'then' and 'else'
    std::sort(incomingVals.begin(), incomingVals.end());
    ASSERT_EQ(incomingVals[0], 1);
    ASSERT_EQ(incomingVals[1], 2);
    std::sort(incomingBlocks.begin(), incomingBlocks.end());
    ASSERT_EQ(incomingBlocks[0], "else");
    ASSERT_EQ(incomingBlocks[1], "then");
    // Next instruction should be return
    ++it;
    ASSERT_TRUE(llvm::isa<llvm::ReturnInst>(&*it));
}
