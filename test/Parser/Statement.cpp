#include "Common.hpp"

TEST(Parser, VarDeclaration)
{
    PREP_MAIN_PARSER("var a;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, VarStatement)
{
    PREP_MAIN_PARSER("var x: Int = 10;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, LetDeclaration)
{
    PREP_MAIN_PARSER("let a = 8;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, LetStatement)
{
    PREP_MAIN_PARSER("let x: Int = 10;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ReturnStatement)
{
    PREP_MAIN_PARSER("return 42;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, IfStatement)
{
    PREP_MAIN_PARSER("if true { return; } else { break; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, WhileStatement)
{
    PREP_MAIN_PARSER("while x < 10 { x = x + 1; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ForStatement)
{
    PREP_MAIN_PARSER("for item in collection { continue; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatement)
{
    PREP_MAIN_PARSER("x = 5;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatementFields)
{
    PREP_MAIN_PARSER("x.field.subfield = 5;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatementArray)
{
    PREP_MAIN_PARSER("x[0] = 5;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatementPtr)
{
    char const *s = R"(
        func mySwap(a: *Int, b: *Int) {
            let tmp = a.*;
            a.* = b.*;
            b.* = tmp;
        }
    )";

    PREP_PARSER(s);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatementFieldsArrayComplex)
{
    PREP_MAIN_PARSER("x.field[i * 2 + 1].subfield = z * 4 * fct();");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, BlockStatement)
{
    PREP_MAIN_PARSER("{ let x = 1; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ExpressionStatement)
{
    PREP_MAIN_PARSER("var a = x + y;");
    EXPECT_TRUE(parser.parse());
}
