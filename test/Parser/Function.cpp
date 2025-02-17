#include "Common.hpp"

TEST(Parser, FunctionDeclarationSimple)
{
    PREP_PARSER("func f() {}");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionDeclarationWithParameters)
{
    PREP_PARSER("func f(a: Int, b: Float) -> Bool { return true; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionDeclarationWithParameters2)
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
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionDeclarationWithAttributesAndTemplate)
{
    char const *str = R"(
        @inline func f<T>(a: Int, b: Float = 3.14) -> Int {
            return a * b;
        }
    )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FunctionDeclarationWithTemplateList)
{
    char const *str = R"(
        func f<T, U, V>(a: Int, b: Float = 3.14) -> Int {
            return ((a as T) * (b as U)) as V;
        }

        func f<T, U,>() {}
    )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}
