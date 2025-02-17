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

#define PREP_MAIN_PARSER(str) PREP_PARSER("func main() {" str "}")

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

// --- Tests for statements ---
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

// --- Tests for expressions ---
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
    PREP_MAIN_PARSER("return x as float;");
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

TEST(Parser, FunctionCallWithTemplateArguments)
{
    PREP_MAIN_PARSER("f::<Int>(1);");
    EXPECT_TRUE(parser.parse());
}

// --- Tests pour les types ---
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

// --- Error tests ---
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
    PREP_MAIN_PARSER("Int[;"); // invalid syntax for an array type
    EXPECT_FALSE(parser.parse());
}

TEST(Parser, ErrorInvalidExpression)
{
    PREP_MAIN_PARSER("1 + ;"); // incomplete expression
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
