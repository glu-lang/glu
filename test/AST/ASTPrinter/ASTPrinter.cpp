#include "ASTPrinter.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Decls.hpp"
#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string str;
    llvm::raw_string_ostream os;
    ASTPrinter printer;

    ASTPrinterTest() : os(str), printer(os) { }
};

class TestNode : public ASTNode {
public:
    TestNode(NodeKind kind, glu::SourceLocation loc, TestNode *parent = nullptr)
        : ASTNode(kind, loc, parent)
    {
    }
};

TEST_F(ASTPrinterTest, PrintNullASTNode)
{
    printer.visit(nullptr);
    EXPECT_EQ(str, "Null ASTNode\n");
}

TEST_F(ASTPrinterTest, PrintNullExprASTNode)
{
    ExprBase *node = nullptr;

    printer.visit(node);
    EXPECT_EQ(str, "Null ASTNode\n");
}

// TEST_F(CompoundStmtTest, PrintCompoundStmt) {
//     ExpressionStmt ExprStmt(LiteralExpr(42, loc));
//     ReturnStmt retStmt(loc, LiteralExpr(0, loc));
//     CompoundStmt compStmt(loc, {exprStmt, returnStmt});

//     printer.visit(&stmt);

//     EXPECT_FALSE(str.empty());
// }

TEST_F(ASTPrinterTest, PrintSimpleASTNode)
{
    BinaryOpExpr binNode(SourceLocation(10));
    ExpressionStmt node(SourceLocation(42), &binNode);
    printer.visit(&node);
    EXPECT_EQ(str, "ExpressionStmt at loc : 42\n  BinaryOpExpr at loc : 10\n");
}

// TEST_F(ASTPrinterTest, PrintIfStmt)
// {
//     IfStmt ifStmt(SourceLocation(10), nullptr, nullptr, nullptr);
//     printer.visit(&ifStmt);
//     EXPECT_EQ(str, "IfStmt at loc : 10\n\n");
// }

// TEST_F(ASTPrinterTest, PrintWhileStmt)
// {
//     WhileStmt whileStmt(SourceLocation(20), nullptr, nullptr);
//     printer.visit(&whileStmt);
//     EXPECT_EQ(str, "WhileStmt at loc : 20\n\n");
// }

// TEST_F(ASTPrinterTest, PrintBreakStmt)
// {
//     BreakStmt breakStmt(SourceLocation(30));
//     printer.visit(&breakStmt);
//     EXPECT_EQ(str, "BreakStmt at loc : 30\n\n");
// }

// TEST_F(ASTPrinterTest, PrintContinueStmt)
// {
//     ContinueStmt continueStmt(SourceLocation(35));
//     printer.visit(&continueStmt);
//     EXPECT_EQ(str, "ContinueStmt at loc : 35\n\n");
// }

// TEST_F(ASTPrinterTest, PrintReturnStmt)
// {
//     ReturnStmt returnStmt(SourceLocation(50), nullptr);
//     printer.visit(&returnStmt);
//     EXPECT_EQ(str, "ReturnStmt at loc : 50\n\n");
// }

} // namespace glu::ast
