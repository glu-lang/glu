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

// --- Tests for top-level definitions ---
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

TEST(Parser, StructDeclaration)
{
    PREP_PARSER("struct S { a: Int, b: Float };");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithAttributes)
{
    PREP_PARSER("@packed struct S { a: Int, b: Float };");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithAttributesAndDefaultValues)
{
    char const *str = R"(
        @packed struct Test {
            a: Int,
            b: Bool = false
        };
        )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, StructDeclarationWithTemplate)
{
    char const *str = R"(
        struct Node<T> {
            value: T,
            next: Node<T>
        };
        )";
    PREP_PARSER(str);
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, EnumDeclaration)
{
    PREP_PARSER("enum E: Int { A, B = 2 };");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, TypealiasDeclaration)
{
    PREP_PARSER("typealias T = Int;");
    EXPECT_TRUE(parser.parse());
}

// TEST(Parser, FunctionDeclarationSimple)
// {
//     PREP_PARSER("func f() {}");
//     EXPECT_TRUE(parser.parse());
// }

// TEST(Parser, FunctionDeclarationWithParameters)
// {
//     PREP_PARSER("func f(a: Int, b: Float) -> Bool { return true; }");
//     EXPECT_TRUE(parser.parse());
// }

// TEST(Parser, FunctionDeclarationWithParameters2)
// {
//     char const *str = R"(
//         func test(a: Int) -> Bool {
//             // This is a comment
//             if (a * 7 + 3 == 0) {
//                 return true;
//             }
//             return false;
//         }
//         )";
//     PREP_PARSER(str);
//     EXPECT_TRUE(parser.parse());
// }

// TEST(Parser, FunctionDeclarationWithAttributesAndTemplate)
// {
//     char const *str = R"(
//         @inline func f<T>(a: Int, b: Float = 3.14) -> Int {
//             return a * b;
//         }
//         )";
//     PREP_PARSER(str);
//     EXPECT_TRUE(parser.parse());
// }

// --- Tests for statements ---
TEST(Parser, VarDeclaration)
{
    PREP_PARSER("var a;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, VarStatement)
{
    PREP_PARSER("var x: Int = 10;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, LetDeclaration)
{
    PREP_PARSER("let a = 8;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, LetStatement)
{
    PREP_PARSER("let x: Int = 10;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ReturnStatement)
{
    PREP_PARSER("return 42;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, IfStatement)
{
    PREP_PARSER("if true { return; } else { break; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, WhileStatement)
{
    PREP_PARSER("while x < 10 { x = x + 1; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ForStatement)
{
    PREP_PARSER("for item in collection { continue; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, AssignmentStatement)
{
    PREP_PARSER("x = 5;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, BlockStatement)
{
    PREP_PARSER("{ let x = 1; }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ExpressionStatement)
{
    PREP_PARSER("x + y;");
    EXPECT_TRUE(parser.parse());
}

// --- Tests for expressions ---
TEST(Parser, BinaryExpression)
{
    PREP_PARSER("1 + 2 * 3;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, UnaryExpression)
{
    PREP_PARSER("-42;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, ParenExpression)
{
    PREP_PARSER("(1 + 2) * 3;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, InitializerList)
{
    PREP_PARSER("{ 1, 2, 3 }");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, TernaryExpression)
{
    PREP_PARSER("x ? 1 : 0;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, FieldAccess)
{
    PREP_PARSER("obj.field;");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, SubscriptExpression)
{
    PREP_PARSER("arr[0];");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, CastExpression)
{
    PREP_PARSER("x as float;");
    EXPECT_TRUE(parser.parse());
}

// TEST(Parser, FunctionCall)
// {
//     PREP_PARSER("f(1, 2);");
//     EXPECT_TRUE(parser.parse());
// }

// TEST(Parser, FunctionCallWithTemplateArguments)
// {
//     PREP_PARSER("f<Int>(1);");
//     EXPECT_TRUE(parser.parse());
// }

// --- Tests pour les types ---
TEST(Parser, SimpleType)
{
    PREP_PARSER("Int;");
    EXPECT_TRUE(parser.parse());
}

// TEST(Parser, FunctionType)
// {
//     PREP_PARSER(R"(
//         @packed struct S {
//             a: (Int, Float) -> Bool,
//             b: Float,
//         };
//     )");
//     EXPECT_TRUE(parser.parse());
// }

TEST(Parser, ArrayType)
{
    PREP_PARSER("Int[10];");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, PointerType)
{
    PREP_PARSER("*unique Int;");
    EXPECT_TRUE(parser.parse());
}

// --- Tests for other alternatives of the start symbol ---
// These tests directly pass an instruction, an expression, or a type without
// a final semicolon.
TEST(Parser, ExpressionWithoutSemicolon)
{
    PREP_PARSER("x + y");
    EXPECT_TRUE(parser.parse());
}

TEST(Parser, TypeWithoutSemicolon)
{
    PREP_PARSER("Int");
    EXPECT_TRUE(parser.parse());
}

// --- Error tests ---
TEST(Parser, ErrorMissingSemicolon)
{
    PREP_PARSER("let a");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorUnexpectedToken)
{
    PREP_PARSER("return return;");
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidFunctionDeclaration)
{
    PREP_PARSER("func (a, b) {}"); // missing function identifier
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidType)
{
    PREP_PARSER("Int[;"); // invalid syntax for an array type
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidExpression)
{
    PREP_PARSER("1 + ;"); // incomplete expression
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidLetDeclaration)
{
    PREP_PARSER("let x: Int;"); // missing initialization
    EXPECT_FALSE(parser.parse());
}
