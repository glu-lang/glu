#include "Scanner.hpp"

#include <gtest/gtest.h>

#define PREP_SCANNER(str)                         \
    std::unique_ptr<llvm::MemoryBuffer> buf(      \
        llvm::MemoryBuffer::getMemBufferCopy(str) \
    );                                            \
    glu::Scanner scanner(buf.get())

#define EXPECT_TOKEN(kind, text)                          \
    do {                                                  \
        glu::Token token = scanner.nextToken();           \
        EXPECT_EQ(token.getKind(), glu::TokenKind::kind); \
        EXPECT_EQ(token.getLexeme(), text);               \
    } while (0)

TEST(Scanner, plain_ident)
{
    PREP_SCANNER("a test string");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(identTok, "test");
    EXPECT_TOKEN(identTok, "string");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, ticked_ident)
{
    PREP_SCANNER("`a` `t#$-=st` `st``r}ng`");
    EXPECT_TOKEN(identTok, "`a`");
    EXPECT_TOKEN(identTok, "`t#$-=st`");
    EXPECT_TOKEN(identTok, "`st``r}ng`");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, decimal_int_lit)
{
    PREP_SCANNER("0 1 123 1234567890 042");
    EXPECT_TOKEN(intLitTok, "0");
    EXPECT_TOKEN(intLitTok, "1");
    EXPECT_TOKEN(intLitTok, "123");
    EXPECT_TOKEN(intLitTok, "1234567890");
    EXPECT_TOKEN(intLitTok, "042");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, hex_int_lit)
{
    PREP_SCANNER("0x0 0x1 0x1f3 0x1234567890ABCDEF 0x01ac");
    EXPECT_TOKEN(intLitTok, "0x0");
    EXPECT_TOKEN(intLitTok, "0x1");
    EXPECT_TOKEN(intLitTok, "0x1f3");
    EXPECT_TOKEN(intLitTok, "0x1234567890ABCDEF");
    EXPECT_TOKEN(intLitTok, "0x01ac");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, bin_int_lit)
{
    PREP_SCANNER("0b0 0b1 0b101");
    EXPECT_TOKEN(intLitTok, "0b0");
    EXPECT_TOKEN(intLitTok, "0b1");
    EXPECT_TOKEN(intLitTok, "0b101");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, float_lit)
{
    PREP_SCANNER("0.0 1.0 42.123");
    EXPECT_TOKEN(floatLitTok, "0.0");
    EXPECT_TOKEN(floatLitTok, "1.0");
    EXPECT_TOKEN(floatLitTok, "42.123");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, string_lit)
{
    PREP_SCANNER("\"\" \"a\" \"test string\\n\" \"\\\"\"");
    EXPECT_TOKEN(stringLitTok, "\"\"");
    EXPECT_TOKEN(stringLitTok, "\"a\"");
    EXPECT_TOKEN(stringLitTok, "\"test string\\n\"");
    EXPECT_TOKEN(stringLitTok, "\"\\\"\"");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, line_comment)
{
    PREP_SCANNER("a // test string\nb");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, block_comment)
{
    PREP_SCANNER("a /* test string */ b");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, block_comment_nested)
{
    PREP_SCANNER("a /* test /* nested */ string */ b");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, unknown_char)
{
    PREP_SCANNER("a # b");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(unknownCharErrorTok, "#");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, unterminated_block_comment)
{
    PREP_SCANNER("a /* test string");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(unterminatedBlockCommentErrorTok, "/* test string");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, unterminated_string_lit)
{
    PREP_SCANNER("a \"test string");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(unterminatedStringLitErrorTok, "\"test string");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, keyword_sample)
{
    PREP_SCANNER("if true return");
    EXPECT_TOKEN(ifKwTok, "if");
    EXPECT_TOKEN(trueKwTok, "true");
    EXPECT_TOKEN(returnKwTok, "return");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, keyword_all)
{
    char const *str = (
#define GLU_KEYWORD(X) #X "\n"
#include "Basic/TokenKind.def"
    );
    PREP_SCANNER(str);
#define GLU_KEYWORD(X) EXPECT_TOKEN(X##KwTok, #X);
#include "Basic/TokenKind.def"
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, punctuators_operators_all)
{
    char const *str = (
#define OPERATOR(X, s) s "\n"
#define PUNCTUATOR(X, s) s "\n"
#include "Basic/TokenKind.def"
    );
    PREP_SCANNER(str);
#define OPERATOR(X, s) EXPECT_TOKEN(X##OpTok, s);
#define PUNCTUATOR(X, s) EXPECT_TOKEN(X##Tok, s);
#include "Basic/TokenKind.def"
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, example_func)
{
    char const *str = R"(
    func test(a: Int) -> Bool {
        // This is a comment
        if (a * 7 + 3 == 0) {
            return true;
        }
        return false;
    }
    )";
    PREP_SCANNER(str);
    EXPECT_TOKEN(funcKwTok, "func");
    EXPECT_TOKEN(identTok, "test");
    EXPECT_TOKEN(lParenTok, "(");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(colonTok, ":");
    EXPECT_TOKEN(identTok, "Int");
    EXPECT_TOKEN(rParenTok, ")");
    EXPECT_TOKEN(arrowTok, "->");
    EXPECT_TOKEN(identTok, "Bool");
    EXPECT_TOKEN(lBraceTok, "{");
    EXPECT_TOKEN(ifKwTok, "if");
    EXPECT_TOKEN(lParenTok, "(");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(mulOpTok, "*");
    EXPECT_TOKEN(intLitTok, "7");
    EXPECT_TOKEN(plusOpTok, "+");
    EXPECT_TOKEN(intLitTok, "3");
    EXPECT_TOKEN(eqOpTok, "==");
    EXPECT_TOKEN(intLitTok, "0");
    EXPECT_TOKEN(rParenTok, ")");
    EXPECT_TOKEN(lBraceTok, "{");
    EXPECT_TOKEN(returnKwTok, "return");
    EXPECT_TOKEN(trueKwTok, "true");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(rBraceTok, "}");
    EXPECT_TOKEN(returnKwTok, "return");
    EXPECT_TOKEN(falseKwTok, "false");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(rBraceTok, "}");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, example_struct)
{
    char const *str = R"(
    @packed struct Test {
        a: Int,
        b: Bool = false,
    }
    )";
    PREP_SCANNER(str);
    EXPECT_TOKEN(atTok, "@");
    EXPECT_TOKEN(identTok, "packed");
    EXPECT_TOKEN(structKwTok, "struct");
    EXPECT_TOKEN(identTok, "Test");
    EXPECT_TOKEN(lBraceTok, "{");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(colonTok, ":");
    EXPECT_TOKEN(identTok, "Int");
    EXPECT_TOKEN(commaTok, ",");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(colonTok, ":");
    EXPECT_TOKEN(identTok, "Bool");
    EXPECT_TOKEN(equalTok, "=");
    EXPECT_TOKEN(falseKwTok, "false");
    EXPECT_TOKEN(commaTok, ",");
    EXPECT_TOKEN(rBraceTok, "}");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, example_import_and_template)
{
    char const *s = R"(
        import templated::f;
        f::<Int>(1);
    )";
    PREP_SCANNER(s);
    EXPECT_TOKEN(importKwTok, "import");
    EXPECT_TOKEN(identTok, "templated");
    EXPECT_TOKEN(coloncolonTok, "::");
    EXPECT_TOKEN(identTok, "f");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(identTok, "f");
    EXPECT_TOKEN(coloncolonLtTok, "::<");
    EXPECT_TOKEN(identTok, "Int");
    EXPECT_TOKEN(gtOpTok, ">");
    EXPECT_TOKEN(lParenTok, "(");
    EXPECT_TOKEN(intLitTok, "1");
    EXPECT_TOKEN(rParenTok, ")");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(eofTok, "");
}

TEST(Scanner, example_pointer_and_dereferencement)
{
    char const *s = R"(
        func mySwap(a: *Int, b: *Int) {
            let tmp = a.*;
            a.* = b.*;
            b.* = tmp;
        }
    )";
    PREP_SCANNER(s);
    EXPECT_TOKEN(funcKwTok, "func");
    EXPECT_TOKEN(identTok, "mySwap");
    EXPECT_TOKEN(lParenTok, "(");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(colonTok, ":");
    EXPECT_TOKEN(mulOpTok, "*");
    EXPECT_TOKEN(identTok, "Int");
    EXPECT_TOKEN(commaTok, ",");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(colonTok, ":");
    EXPECT_TOKEN(mulOpTok, "*");
    EXPECT_TOKEN(identTok, "Int");
    EXPECT_TOKEN(rParenTok, ")");
    EXPECT_TOKEN(lBraceTok, "{");
    EXPECT_TOKEN(letKwTok, "let");
    EXPECT_TOKEN(identTok, "tmp");
    EXPECT_TOKEN(equalTok, "=");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(derefOpTok, ".*");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(identTok, "a");
    EXPECT_TOKEN(derefOpTok, ".*");
    EXPECT_TOKEN(equalTok, "=");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(derefOpTok, ".*");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(identTok, "b");
    EXPECT_TOKEN(derefOpTok, ".*");
    EXPECT_TOKEN(equalTok, "=");
    EXPECT_TOKEN(identTok, "tmp");
    EXPECT_TOKEN(semiTok, ";");
    EXPECT_TOKEN(rBraceTok, "}");
    EXPECT_TOKEN(eofTok, "");
}
