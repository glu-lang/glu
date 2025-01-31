#include "Scanner.hpp"

#include <gtest/gtest.h>
#include <sstream>

using namespace glu;

#define PREP_SCANNER(scanner, str)                \
    std::unique_ptr<llvm::MemoryBuffer> buf(      \
        llvm::MemoryBuffer::getMemBufferCopy(str) \
    );                                            \
    Scanner scanner(buf.get())

#define EXPECT_TOKEN(scanner, kind, text)   \
    do {                                    \
        Token token = scanner.nextToken();  \
        EXPECT_EQ(token.getKind(), kind);   \
        EXPECT_EQ(token.getLexeme(), text); \
    } while (0)

TEST(Scanner, plain_ident)
{
    PREP_SCANNER(scanner, "a test string");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "test");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "string");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, ticked_ident)
{
    PREP_SCANNER(scanner, "`a` `t#$-=st` `st``r}ng`");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "`a`");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "`t#$-=st`");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "`st``r}ng`");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, decimal_int_lit)
{
    PREP_SCANNER(scanner, "0 1 123 1234567890 042");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "1");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "123");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "1234567890");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "042");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, hex_int_lit)
{
    PREP_SCANNER(scanner, "0x0 0x1 0x1f3 0x1234567890ABCDEF 0x01ac");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0x0");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0x1");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0x1f3");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0x1234567890ABCDEF");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0x01ac");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, bin_int_lit)
{
    PREP_SCANNER(scanner, "0b0 0b1 0b101");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0b0");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0b1");
    EXPECT_TOKEN(scanner, TokenKind::intLitTok, "0b101");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, float_lit)
{
    PREP_SCANNER(scanner, "0.0 1.0 42.123");
    EXPECT_TOKEN(scanner, TokenKind::floatLitTok, "0.0");
    EXPECT_TOKEN(scanner, TokenKind::floatLitTok, "1.0");
    EXPECT_TOKEN(scanner, TokenKind::floatLitTok, "42.123");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, string_lit)
{
    PREP_SCANNER(scanner, "\"\" \"a\" \"test string\\n\" \"\\\"\"");
    EXPECT_TOKEN(scanner, TokenKind::stringLitTok, "\"\"");
    EXPECT_TOKEN(scanner, TokenKind::stringLitTok, "\"a\"");
    EXPECT_TOKEN(scanner, TokenKind::stringLitTok, "\"test string\\n\"");
    EXPECT_TOKEN(scanner, TokenKind::stringLitTok, "\"\\\"\"");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, line_comment)
{
    PREP_SCANNER(scanner, "a // test string\nb");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "b");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, block_comment)
{
    PREP_SCANNER(scanner, "a /* test string */ b");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "b");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, block_comment_nested)
{
    PREP_SCANNER(scanner, "a /* test /* nested */ string */ b");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "b");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, unknown_char)
{
    PREP_SCANNER(scanner, "a # b");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(scanner, TokenKind::unknownCharErrorTok, "#");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "b");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, unterminated_block_comment)
{
    PREP_SCANNER(scanner, "a /* test string");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(
        scanner, TokenKind::unterminatedBlockCommentErrorTok, "/* test string"
    );
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, unterminated_string_lit)
{
    PREP_SCANNER(scanner, "a \"test string");
    EXPECT_TOKEN(scanner, TokenKind::identTok, "a");
    EXPECT_TOKEN(
        scanner, TokenKind::unterminatedStringLitErrorTok, "\"test string"
    );
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, keyword_sample)
{
    PREP_SCANNER(scanner, "if true return");
    EXPECT_TOKEN(scanner, TokenKind::ifKwTok, "if");
    EXPECT_TOKEN(scanner, TokenKind::trueKwTok, "true");
    EXPECT_TOKEN(scanner, TokenKind::returnKwTok, "return");
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}

TEST(Scanner, keyword_all)
{
    char const *str = (
#define GLU_KEYWORD(X) #X "\n"
#include "Basic/TokenKind.def"
    );
    PREP_SCANNER(scanner, str);
#define GLU_KEYWORD(X) EXPECT_TOKEN(scanner, TokenKind::X##KwTok, #X);
#include "Basic/TokenKind.def"
    EXPECT_TOKEN(scanner, TokenKind::eofTok, "");
}
