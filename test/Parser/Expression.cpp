#include "Common.hpp"

TEST(Parser, BinaryExpression)
{
    PREP_MAIN_PARSER("var a = 1 + 2 * 3;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, UnaryExpression)
{
    PREP_MAIN_PARSER("var a = -42;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ParenExpression)
{
    PREP_MAIN_PARSER("var a = (1 + 2) * 3;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, InitializerList)
{
    PREP_MAIN_PARSER("let list = { 1, 2, 3 };");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, TernaryExpression)
{
    PREP_MAIN_PARSER(R"(
        var a = x ? 1 : 0;
        var b = x ? 1 : y ? 2 : 0;
        let c = x == 0 ? fonction1() : fonction2();
        return x == 0 ? fonction1() : fonction2();
    )");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FieldAccess)
{
    PREP_MAIN_PARSER("var a = obj.field;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, SubscriptExpression)
{
    PREP_MAIN_PARSER("var a = arr[0];");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, CastExpression)
{
    PREP_MAIN_PARSER("return x as Float;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionCall)
{
    PREP_MAIN_PARSER("f(1);");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionCallWithManyParameters)
{
    PREP_MAIN_PARSER("add(1, 3);");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionCallFromOtherNamespace)
{
    PREP_MAIN_PARSER("std::std::std::std::exit(42);");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionCallWithTemplateArguments)
{
    PREP_MAIN_PARSER("f::<Int>(1);");
    EXPECT_TRUE(parser.parse());
}
