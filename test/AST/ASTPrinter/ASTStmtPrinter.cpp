#include "AST/ASTContext.hpp"
#include "AST/ASTPrinter.hpp"
#include "Basic/SourceManager.hpp"
#include "Basic/Tokens.hpp"
#include "Lexer/Scanner.hpp"
#include "llvm/Support/MemoryBuffer.h"
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string str;
    glu::SourceManager sm;
    glu::ast::ASTContext ctx;
    llvm::raw_string_ostream out;
    glu::TypedMemoryArena<glu::ast::ASTNode> &ast;

    ASTPrinterTest() : out(str), ast(ctx.getASTMemoryArena()) { }
};

TEST_F(ASTPrinterTest, PrintAssignStmt)
{
    std::string stmt = "x = 42";
    std::unique_ptr<llvm::MemoryBuffer> buf(
        llvm::MemoryBuffer::getMemBufferCopy(stmt)
    );
    sm.loadBuffer(std::move(buf), "test.glu");

    glu::Token assignToken(glu::TokenKind::equalTok, "=");
    auto lhs = ast.create<glu::ast::RefExpr>(glu::SourceLocation(1), "x");
    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto rhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(3)
    );
    glu::ast::AssignStmt node(glu::SourceLocation(0), lhs, assignToken, rhs);

    node.print(&sm, out);

    EXPECT_EQ(
        out.str(),
        "AssignStmt at file : test.glu line : 1 col : 1\n"
        "  -->equal assignement with: \n"
        "  RefExpr at file : test.glu line : 1 col : 2\n"
        "    -->Reference to: x\n"
        "  LiteralExpr at file : test.glu line : 1 col : 4\n"
        "    -->Integer: 42\n"
    );
}
