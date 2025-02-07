#include "Parser.hpp"
#include "Scanner.hpp"

#include <gtest/gtest.h>
#include <memory>

#define PREP_PARSER(str)                          \
    std::unique_ptr<llvm::MemoryBuffer> buf(      \
        llvm::MemoryBuffer::getMemBufferCopy(str) \
    );                                            \
    glu::Scanner scanner(buf.get());              \
    glu::Parser parser(scanner /*, 1*/)

TEST(Parser, EmptyInput)
{
    PREP_PARSER("");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, SimpleFunction)
{
    PREP_PARSER("func test() { }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, VarDeclaration)
{
    PREP_PARSER("let a;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclaration)
{
    PREP_PARSER("struct S { let a; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, Error)
{
    PREP_PARSER("let a");
    EXPECT_FALSE(parser.parse());
}
