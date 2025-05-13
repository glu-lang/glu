#include "GILPrinter.hpp"
#include "Types.hpp"

#include <gtest/gtest.h>

using namespace glu::gil;

class GILPrinterTest : public ::testing::Test {
protected:
    std::string str;
    llvm::raw_string_ostream os;
    glu::SourceManager sm;
    GILPrinter printer;
    llvm::BumpPtrAllocator alloc;

    GILPrinterTest() : os(str), printer(sm, os) { }
};

#define PREP_SM(str, file)                        \
    sm.reset();                                   \
    std::unique_ptr<llvm::MemoryBuffer> buf(      \
        llvm::MemoryBuffer::getMemBufferCopy(str) \
    );                                            \
    sm.loadBuffer(std::move(buf), file);

#define PREP_MAIN_SM(str) PREP_SM("func main() {" str "}", "main.glu")

TEST_F(GILPrinterTest, IntegerLiteralInst)
{
    auto inst = IntegerLiteralInst::create(alloc, Type(), llvm::APInt(32, 42));
    printer.visit(inst);
    EXPECT_EQ(str, "%<unknown> = integer_literal $, 42\n");
}

TEST_F(GILPrinterTest, SimpleFunction)
{
    auto inst = IntegerLiteralInst::create(alloc, Type(), llvm::APInt(32, 42));
    auto bb = BasicBlock::create(alloc, "bb0", {});
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

TEST_F(GILPrinterTest, FunctionWithArguments)
{
    auto ty = new glu::types::FloatTy(glu::types::FloatTy::DOUBLE);
    auto bb = BasicBlock::create(alloc, "", { ty });
    auto fn = new Function("test", nullptr);
    fn->addBasicBlockAtEnd(bb);
    auto fl = FloatLiteralInst::create(alloc, Type(), llvm::APFloat(42.5));
    bb->getInstructions().push_back(fl);
    bb->getInstructions().push_back(
        CallInst::create(
            alloc, fn,
            std::vector<Value> { bb->getArgument(0), fl->getResult(0) }
        )
    );
    printer.visit(fn);
    EXPECT_EQ(str, R"(gil @test : $ {
bb0(%0 : $):
    %1 = float_literal $, 42.5
    %2 = call @test, %0 : $, %1 : $
}

)");
    delete ty;
    delete fn;
}

TEST_F(GILPrinterTest, DebugInstTest)
{
    PREP_SM(
        R"(
        func test() { let x = 10; let y = 20; }";
    )",
        "main.glu"
    );

    auto inst = IntegerLiteralInst::create(alloc, Type(), llvm::APInt(32, 10));
    inst->setLocation(glu::SourceLocation(1));

    auto debugInst = new DebugInst(
        "x", inst->getResult(0), DebugBindingType::Let, inst->getLocation()
    );

    auto bb = BasicBlock::create(alloc, "bb0", {});
    auto fn = new Function("test", nullptr);
    fn->addBasicBlockAtEnd(bb);
    bb->getInstructions().push_back(inst);
    bb->getInstructions().push_back(debugInst);

    printer.visit(fn);

    EXPECT_EQ(str, R"(gil @test : $ {
bb0:
    %0 = integer_literal $, 10, loc "main.glu":2:1
    debug %0 : $, let "x", loc "main.glu":2:1
}

)");

    delete debugInst;
    delete fn;
}
