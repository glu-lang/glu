#include "Common.hpp"

TEST(Parser, ImportDeclarationSimple)
{
    PREP_PARSER("import MyModule;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ImportDeclarationWithNamespace)
{
    PREP_PARSER("import A::B;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ImportDeclarationWithWildcard)
{
    PREP_PARSER("import A::*;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ImportDeclarationWithList)
{
    PREP_PARSER("import A::{B, C};");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ImportDeclarationWithAlias)
{
    PREP_PARSER("import A::B as C;");
    EXPECT_TRUE(parser.parse());
}
