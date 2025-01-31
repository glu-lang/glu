#include "Scanner.hpp"

#include <gtest/gtest.h>
#include <sstream>

using namespace glu;

TEST(Scanner, plain_ident)
{
    std::stringstream stream("a test string");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "a");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "test");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "string");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, ticked_ident)
{
    std::stringstream stream("`a` `t#$-=st` `st``r}ng`");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "`a`");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "`t#$-=st`");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "`st``r}ng`");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, decimal_int_lit)
{
    std::stringstream stream("0 1 123 1234567890 042");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "1");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "123");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "1234567890");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "042");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, hex_int_lit)
{
    std::stringstream stream("0x0 0x1 0x1f3 0x1234567890ABCDEF 0x01ac");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0x0");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0x1");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0x1f3");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0x1234567890ABCDEF");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0x01ac");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, bin_int_lit)
{
    std::stringstream stream("0b0 0b1 0b101");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0b0");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0b1");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::intLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0b101");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, float_lit)
{
    std::stringstream stream("0.0 1.0 42.123");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::floatLitTok);
    EXPECT_EQ(scanner.getTokenText(), "0.0");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::floatLitTok);
    EXPECT_EQ(scanner.getTokenText(), "1.0");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::floatLitTok);
    EXPECT_EQ(scanner.getTokenText(), "42.123");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, string_lit)
{
    std::stringstream stream("\"\" \"a\" \"test string\\n\" \"\\\"\"");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::stringLitTok);
    EXPECT_EQ(scanner.getTokenText(), "\"\"");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::stringLitTok);
    EXPECT_EQ(scanner.getTokenText(), "\"a\"");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::stringLitTok);
    EXPECT_EQ(scanner.getTokenText(), "\"test string\\n\"");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::stringLitTok);
    EXPECT_EQ(scanner.getTokenText(), "\"\\\"\"");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, line_comment)
{
    std::stringstream stream("a // test string\nb");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "a");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "b");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, block_comment)
{
    std::stringstream stream("a /* test string */ b");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "a");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "b");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}

TEST(Scanner, block_comment_nested)
{
    std::stringstream stream("a /* test /* nested */ string */ b");
    Scanner scanner(&stream);
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "a");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::identTok);
    EXPECT_EQ(scanner.getTokenText(), "b");
    EXPECT_EQ(scanner.getNextToken(), TokenKind::eofTok);
}
