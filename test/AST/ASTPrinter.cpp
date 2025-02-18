#include "ASTPrinter.hpp"
#include "Basic/SourceLocation.hpp"
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

class TestNode : public glu::ast::ASTNode {
public:
    TestNode(
        glu::ast::NodeKind kind, glu::SourceLocation loc,
        TestNode *parent = nullptr
    )
        : glu::ast::ASTNode(kind, loc, parent)
    {
    }
};

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string str;
    llvm::raw_string_ostream os;
    glu::ast::ASTPrinter printer;

    ASTPrinterTest() : os(str), printer(os) { }
};

// Test Google Test
TEST_F(ASTPrinterTest, PrintsSimpleNode)
{
    glu::SourceLocation loc(1);
    TestNode node(glu::ast::NodeKind::ReturnStmtKind, loc);

    printer.visit(&node);

    EXPECT_EQ(str, "ReturnStmt\n");
}

// TEST_F(ASTPrinterTest, PrintsTwoNodes) {
//     glu::SourceLocation firstLoc(1);
//     glu::SourceLocation secondLoc(2);
//     TestNode firstNode(glu::ast::NodeKind::ReturnStmtKind, firstLoc);
//     TestNode secondNode(glu::ast::NodeKind::AssignStmtKind, secondLoc,
//         &firstNode);

//     printer.visit(&secondNode);

//     EXPECT_EQ(str, "ReturnStmt\n  AssignStmt\n");
// }
