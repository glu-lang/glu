#include "Optimizer/AnalysisPasses/ValueUseChecker.hpp"

#include "AST/ASTContext.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/Instructions.hpp"
#include "GIL/Module.hpp"

#include <gtest/gtest.h>

#include <llvm/ADT/APInt.h>

#include <array>
#include <memory>
#include <vector>

using namespace glu;

namespace {

class ValueUseCheckerTest : public ::testing::Test {
protected:
    ast::ASTContext astCtx;
    types::IntTy *intTy = nullptr;
    gil::Type gilIntTy;
    types::FunctionTy *functionTy = nullptr;
    std::unique_ptr<gil::Module> module;

    void SetUp() override
    {
        intTy = astCtx.getTypesMemoryArena().create<types::IntTy>(
            types::IntTy::Signed, 32
        );
        gilIntTy = gil::Type(4, 4, false, intTy);

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

TEST_F(ValueUseCheckerTest, ReportsSingleUseForDirectOperand)
{
    auto *fn = createFunction("single_use");
    auto *entry = appendBlock(fn, "entry");

    auto *literal
        = gil::IntegerLiteralInst::create(gilIntTy, llvm::APInt(32, 7));
    entry->addInstructionAtEnd(literal);

    auto *retInst = new gil::ReturnInst(literal->getResult(0));
    entry->addInstructionAtEnd(retInst);

    EXPECT_TRUE(optimizer::valueIsUsedOnlyBy(literal->getResult(0), retInst));
}

TEST_F(ValueUseCheckerTest, DetectsExtraUsesWithinOperandLists)
{
    auto *fn = createFunction("list_use");
    auto *entry = appendBlock(fn, "entry");

    auto *literal
        = gil::IntegerLiteralInst::create(gilIntTy, llvm::APInt(32, 42));
    entry->addInstructionAtEnd(literal);

    std::array<gil::Type, 1> destArgs { gilIntTy };
    auto *dest = appendBlock(fn, "dest", destArgs);

    auto *branch = gil::BrInst::create(dest, { literal->getResult(0) });
    entry->addInstructionAtEnd(branch);

    auto *retInst = new gil::ReturnInst(literal->getResult(0));
    entry->addInstructionAtEnd(retInst);

    EXPECT_FALSE(optimizer::valueIsUsedOnlyBy(literal->getResult(0), retInst));
}

TEST_F(ValueUseCheckerTest, TracksVariantOperands)
{
    auto *fn = createFunction("variant_use");
    auto *entry = appendBlock(fn, "entry");

    auto *calleeTy = astCtx.getTypesMemoryArena().create<types::FunctionTy>(
        std::vector<types::TypeBase *> {}, intTy
    );
    auto *calleePtrTyBase
        = astCtx.getTypesMemoryArena().create<types::PointerTy>(calleeTy);
    gil::Type calleePtrTy(
        sizeof(void *), alignof(void *), false, calleePtrTyBase
    );

    auto *target = new gil::Function("target", calleeTy, nullptr);
    module->addFunction(target);

    auto *ptrInst = new gil::FunctionPtrInst(target, calleePtrTy);
    entry->addInstructionAtEnd(ptrInst);

    auto *callInst = gil::CallInst::create(gilIntTy, ptrInst->getResult(0), {});
    entry->addInstructionAtEnd(callInst);

    EXPECT_TRUE(optimizer::valueIsUsedOnlyBy(ptrInst->getResult(0), callInst));

    auto *secondCall
        = gil::CallInst::create(gilIntTy, ptrInst->getResult(0), {});
    entry->addInstructionAtEnd(secondCall);

    EXPECT_FALSE(optimizer::valueIsUsedOnlyBy(ptrInst->getResult(0), callInst));
}

} // namespace
