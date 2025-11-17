#include "AST/ASTContext.hpp"
#include "IRDec/ModuleLifter.hpp"

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <gtest/gtest.h>
#include <llvm/Support/Casting.h>

using namespace glu;

class DITypeLifterTest : public ::testing::Test {
protected:
    llvm::LLVMContext ctx;
    llvm::Module module { "test", ctx };
    glu::ast::ASTContext astCtx;
    glu::irdec::ModuleLiftingContext mlc { astCtx };

    DITypeLifterTest() : astCtx() { }
};

TEST_F(DITypeLifterTest, LiftSignedIntBasicType)
{
    llvm::DIBuilder dib(module);
    auto *file = dib.createFile("t", ".");
    dib.createCompileUnit(llvm::dwarf::DW_LANG_C, file, "test", false, "", 0);

    // create a signed 32-bit basic type
    auto *diInt = dib.createBasicType("int32", 32, llvm::dwarf::DW_ATE_signed);

    dib.finalize();

    std::vector<glu::ast::DeclBase *> decls;
    auto *lifted = glu::irdec::lift(diInt, mlc);
    ASSERT_NE(lifted, nullptr);
    auto *intTy = llvm::dyn_cast<types::IntTy>(lifted);
    ASSERT_NE(intTy, nullptr);
    EXPECT_EQ(intTy->getSignedness(), types::IntTy::Signed);
    EXPECT_EQ(intTy->getBitWidth(), 32);
}

TEST_F(DITypeLifterTest, LiftUnsignedIntBasicType)
{
    llvm::DIBuilder dib(module);
    auto *file = dib.createFile("t", ".");
    dib.createCompileUnit(llvm::dwarf::DW_LANG_C, file, "test", false, "", 0);

    auto *diUInt
        = dib.createBasicType("uint16", 16, llvm::dwarf::DW_ATE_unsigned);
    dib.finalize();

    std::vector<glu::ast::DeclBase *> decls;
    auto *lifted = glu::irdec::lift(diUInt, mlc);
    ASSERT_NE(lifted, nullptr);
    auto *intTy = llvm::dyn_cast<types::IntTy>(lifted);
    ASSERT_NE(intTy, nullptr);
    EXPECT_EQ(intTy->getSignedness(), types::IntTy::Unsigned);
    EXPECT_EQ(intTy->getBitWidth(), 16);
}

TEST_F(DITypeLifterTest, LiftFloatBasicType)
{
    llvm::DIBuilder dib(module);
    auto *file = dib.createFile("t", ".");
    dib.createCompileUnit(llvm::dwarf::DW_LANG_C, file, "test", false, "", 0);

    auto *diFloat
        = dib.createBasicType("float32", 32, llvm::dwarf::DW_ATE_float);
    dib.finalize();

    std::vector<glu::ast::DeclBase *> decls;
    auto *lifted = glu::irdec::lift(diFloat, mlc);
    ASSERT_NE(lifted, nullptr);
    auto *floatTy = llvm::dyn_cast<types::FloatTy>(lifted);
    ASSERT_NE(floatTy, nullptr);
    EXPECT_EQ(floatTy->getBitWidth(), 32);
}
