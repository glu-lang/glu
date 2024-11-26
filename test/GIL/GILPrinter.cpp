#include "GILPrinter.hpp"

#include <gtest/gtest.h>

using namespace glu::gil;

TEST(GILPrinterTest, IntegerLiteralInst)
{
    std::string str;
    llvm::raw_string_ostream os(str);
    GILPrinter printer(os);
    auto inst = new IntegerLiteralInst(Type(), llvm::APInt(32, 42));
    printer.visit(inst);
    EXPECT_EQ(str, "%<unknown> = integer_literal $, 42\n");
    delete inst;
}

TEST(GILPrinterTest, Function)
{
    std::string str;
    llvm::raw_string_ostream os(str);
    GILPrinter printer(os);
    auto inst = new IntegerLiteralInst(Type(), llvm::APInt(32, 42));
    auto bb = new BasicBlock();
    auto fn = new Function("test", nullptr);
    fn->addBasicBlockAtEnd(bb);
    bb->getInstructions().push_back(inst);
    printer.visit(fn);
    EXPECT_EQ(str, R"(gil @test : $ {
bb0:
    %0 = integer_literal $, 42
}

)");
    delete fn;
}
