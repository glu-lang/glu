#include "AST/ASTContext.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/Instructions.hpp"
#include "GIL/Module.hpp"
#include "GIL/Value.hpp"

#include <gtest/gtest.h>

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>

#include <vector>

using namespace glu;

namespace {

class ValueReplaceAllUsesWithTest : public ::testing::Test {
protected:
    ast::ASTContext astCtx;
    types::IntTy *intTy = nullptr;
    types::FunctionTy *functionTy = nullptr;
    std::unique_ptr<gil::Module> module;

    void SetUp() override
    {
        intTy = astCtx.getTypesMemoryArena().create<types::IntTy>(
            types::IntTy::Signed, 32
        );

        functionTy = astCtx.getTypesMemoryArena().create<types::FunctionTy>(
            std::vector<types::TypeBase *> {}, intTy
        );

        module = std::make_unique<gil::Module>("test");
    }

    gil::Function *createFunction(llvm::StringRef name)
    {
        auto *fn = new gil::Function(name, functionTy, nullptr);
        module->addFunction(fn);
        return fn;
    }

    gil::BasicBlock *appendBlock(
        gil::Function *fn, llvm::StringRef label,
        llvm::ArrayRef<gil::Type> args = {}
    )
    {
        auto *bb = gil::BasicBlock::create(label, args);
        fn->getBasicBlocks().push_back(bb);
        return bb;
    }
};

TEST_F(ValueReplaceAllUsesWithTest, ReplacesScalarOperandInInstruction)
{
    auto *fn = createFunction("scalar_rauw");
    auto *entry = appendBlock(fn, "entry");

    auto *original = gil::IntegerLiteralInst::create(intTy, llvm::APInt(32, 1));
    auto *replacement
        = gil::IntegerLiteralInst::create(intTy, llvm::APInt(32, 2));

    entry->addInstructionAtEnd(original);
    entry->addInstructionAtEnd(replacement);

    auto *retInst = new gil::ReturnInst(original->getResult(0));
    entry->addInstructionAtEnd(retInst);

    original->getResult(0).replaceAllUsesWith(replacement->getResult(0));

    EXPECT_EQ(retInst->getValue(), replacement->getResult(0));
}

TEST_F(ValueReplaceAllUsesWithTest, ReplacesValuesInsideOperandLists)
{
    auto *fn = createFunction("branch_rauw");
    auto *entry = appendBlock(fn, "entry");

    std::array<gil::Type, 1> destArgs { intTy };
    auto *dest = appendBlock(fn, "dest", destArgs);

    auto *initial = gil::IntegerLiteralInst::create(intTy, llvm::APInt(32, 10));
    auto *updated = gil::IntegerLiteralInst::create(intTy, llvm::APInt(32, 20));

    entry->addInstructionAtEnd(initial);
    entry->addInstructionAtEnd(updated);

    auto *branch = gil::BrInst::create(dest, { initial->getResult(0) });
    entry->addInstructionAtEnd(branch);

    initial->getResult(0).replaceAllUsesWith(updated->getResult(0));

    ASSERT_EQ(branch->getArgs().size(), 1u);
    EXPECT_EQ(branch->getArgs()[0], updated->getResult(0));
}

TEST_F(ValueReplaceAllUsesWithTest, ReplacesCallCalleeWhenPassedAsValue)
{
    auto *fn = createFunction("call_rauw");
    auto *entry = appendBlock(fn, "entry");

    // Prepare auxiliary functions and their pointer type.
    auto *calleeTy = astCtx.getTypesMemoryArena().create<types::FunctionTy>(
        std::vector<types::TypeBase *> {}, intTy
    );
    auto *calleePtrTy
        = astCtx.getTypesMemoryArena().create<types::PointerTy>(calleeTy);

    auto *targetA = new gil::Function("targetA", calleeTy, nullptr);
    auto *targetB = new gil::Function("targetB", calleeTy, nullptr);
    module->addFunction(targetA);
    module->addFunction(targetB);

    auto *ptrInstA = new gil::FunctionPtrInst(targetA, calleePtrTy);
    auto *ptrInstB = new gil::FunctionPtrInst(targetB, calleePtrTy);
    entry->addInstructionAtEnd(ptrInstA);
    entry->addInstructionAtEnd(ptrInstB);

    auto *callInst = gil::CallInst::create(intTy, ptrInstA->getResult(0), {});
    entry->addInstructionAtEnd(callInst);

    ptrInstA->getResult(0).replaceAllUsesWith(ptrInstB->getResult(0));

    auto maybeValue = callInst->getFunctionPtrValue();
    ASSERT_TRUE(maybeValue.has_value());
    EXPECT_EQ(*maybeValue, ptrInstB->getResult(0));
}

} // namespace
