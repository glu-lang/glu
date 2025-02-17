#include "Common.hpp"

TEST(Parser, StructDeclaration)
{
    PREP_PARSER("struct S { a: Int, b: Float }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithAttributes)
{
    PREP_PARSER("@packed struct S { a: Int, b: Float }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithAttributesAndDefaultValues)
{
    char const *str = R"(
        @packed struct Test {
            a: Int,
            b: Bool = false
        }
    )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithTemplate)
{
    char const *str = R"(
        struct Node<T> {
            value: T,
            next: Node<T>,
        }
    )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, EnumDeclaration)
{
    PREP_PARSER("enum E: Int { A, B = 2 }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, TypealiasDeclaration)
{
    PREP_PARSER("typealias T = Int;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, SimpleType)
{
    PREP_MAIN_PARSER("var a: Int;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionType)
{
    PREP_PARSER(R"(
        @packed struct S {
            a: (Int, Float) -> Bool,
            b: Float
        }
    )");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ArrayType)
{
    PREP_MAIN_PARSER("var a: Int[10];");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, PointerType)
{
    PREP_MAIN_PARSER("var ptr: *unique Int;");
    EXPECT_TRUE(parser.parse());
}
