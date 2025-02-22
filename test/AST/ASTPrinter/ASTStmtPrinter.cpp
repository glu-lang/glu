#include "AST/ASTNode.hpp"
#include "AST/Decls.hpp"
#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"
#include "ASTPrinter.hpp"
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string str;
    llvm::raw_string_ostream os;
    ASTPrinter printer;
    SourceManager srcManager;

    ASTPrinterTest() : os(str), printer(&srcManager, os)
    {
        if (!srcManager.loadFile("test/AST/ASTPrinter/Samples/Nodes.glu"))
            llvm::errs() << "Error loading file\n";
    }
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

TEST_F(ASTPrinterTest, PrintSimpleASTNode)
{
    BinaryOpExpr binNode(SourceLocation(1));
    ExpressionStmt exprNode(SourceLocation(2), &binNode);
    printer.visit(&exprNode);
    EXPECT_EQ(str, "ExpressionStmt at loc : 42\n  BinaryOpExpr at loc : 10\n");
}

} // namespace glu::ast
