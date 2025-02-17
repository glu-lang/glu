#include "Common.hpp"

TEST(Parser, ExpressionWithoutSemicolon)
{
    PREP_MAIN_PARSER("x + y"); // missing semicolon
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, TypeWithoutSemicolon)
{
    PREP_MAIN_PARSER("Int");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorMissingSemicolon)
{
    PREP_MAIN_PARSER("let a");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorUnexpectedToken)
{
    PREP_MAIN_PARSER("return return;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidFunctionDeclaration)
{
    PREP_PARSER("func (a, b) {}"); // missing function identifier
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidType)
{
    PREP_MAIN_PARSER("Int[;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidExpression)
{
    PREP_MAIN_PARSER("1 + ;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidLetDeclaration)
{
    PREP_MAIN_PARSER("let x: Int;"); // missing initialization
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidImportPathChaining)
{
    PREP_PARSER("import hello::*::world::{a, b::*::stuff}::help::*;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidVarDeclaration_NoExpression)
{
    PREP_MAIN_PARSER("var a: Int =;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidVarDeclaration_MissingEqualButHasExpression)
{
    PREP_MAIN_PARSER("var a: Int 10;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidVarDeclaration_MissingType)
{
    PREP_MAIN_PARSER("var a 0;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidVarDeclaration_MissingExpression)
{
    PREP_MAIN_PARSER("var a =;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorChainedEqualityExpression)
{
    PREP_MAIN_PARSER("var v = a == b == c;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorChainedRelationalExpression)
{
    PREP_MAIN_PARSER("var v = a < b < c;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorTernaryExpression)
{
    PREP_MAIN_PARSER("a ? func() : funcB();");
    EXPECT_FALSE(parser.parse());
}
