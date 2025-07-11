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

    GILPrinterTest() : os(str), printer(&sm, os) { }
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
    auto intType = new (alloc) glu::types::IntTy(glu::types::IntTy::Signed, 32);
    auto gilType = glu::gil::Type(4, 4, false, intType);
    auto inst = IntegerLiteralInst::create(alloc, gilType, llvm::APInt(32, 42));
    auto bb = BasicBlock::create(alloc, "entry", {});
    auto fn = new Function("test", nullptr);
    fn->addBasicBlockAtEnd(bb);
    bb->getInstructions().push_back(inst);
    printer.visit(fn);
    EXPECT_EQ(str, R"(gil @test : $ {
entry:
    %0 = integer_literal $i32, 42
}

)");
    delete fn;
}

TEST_F(GILPrinterTest, SimpleFunction)
{
    auto intType = new (alloc) glu::types::IntTy(glu::types::IntTy::Signed, 32);
    auto gilType = glu::gil::Type(4, 4, false, intType);
    auto inst = IntegerLiteralInst::create(alloc, gilType, llvm::APInt(32, 42));
    auto bb = BasicBlock::create(alloc, "bb0", {});
    auto fn = new Function("test", nullptr);
    fn->addBasicBlockAtEnd(bb);
    bb->getInstructions().push_back(inst);
    printer.visit(fn);
    EXPECT_EQ(str, R"(gil @test : $ {
bb0:
    %0 = integer_literal $i32, 42
}

)");
    delete fn;
}

TEST_F(GILPrinterTest, FunctionWithArguments)
{
    auto ty = new (alloc) glu::types::FloatTy(glu::types::FloatTy::DOUBLE);
    auto gty = glu::gil::Type(8, 8, true, ty);
    auto fty = glu::types::FunctionTy::create(alloc, { ty }, ty);
    auto bb = BasicBlock::create(alloc, "", { gty });
    auto fn = new (alloc) Function("test", fty);
    fn->addBasicBlockAtEnd(bb);
    auto fl = FloatLiteralInst::create(alloc, gty, llvm::APFloat(42.5));
    bb->getInstructions().push_back(fl);
    bb->getInstructions().push_back(
        CallInst::create(
            alloc, gty, fn,
            std::vector<Value> { bb->getArgument(0), fl->getResult(0) }
        )
    );
    printer.visit(fn);
    EXPECT_EQ(str, R"(gil @test : $(f64) -> f64 {
bb0(%0 : $f64):
    %1 = float_literal $f64, 42.5
    %2 = call @test, %0 : $f64, %1 : $f64
}

)");
}

TEST_F(GILPrinterTest, DebugInstTest)
{
    PREP_SM(
        R"(
        func test() { let x = 10; let y = 20; }";
    )",
        "main.glu"
    );

    auto intType = new (alloc) glu::types::IntTy(glu::types::IntTy::Signed, 32);
    auto gilType = glu::gil::Type(4, 4, false, intType);
    auto inst = IntegerLiteralInst::create(alloc, gilType, llvm::APInt(32, 10));
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
    %0 = integer_literal $i32, 10, loc "main.glu":2:1
    debug %0 : $i32, let "x", loc "main.glu":2:1
}

)");

    delete debugInst;
    delete fn;
}
