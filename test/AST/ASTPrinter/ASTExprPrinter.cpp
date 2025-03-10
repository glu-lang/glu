#include "ASTPrinter.hpp"

TEST_F(ASTPrinterTest, PrintBinaryOp)
{
    PREP_ASTPRINTER("1 + 2", "BinaryOpExpr.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto lhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 1), &intTy, glu::SourceLocation(0)
    );
    auto rhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 2), &intTy, glu::SourceLocation(4)
    );
    glu::Token plusToken(glu::TokenKind::plusOpTok, "+");
    glu::ast::BinaryOpExpr node(glu::SourceLocation(2), lhs, plusToken, rhs);

    node.print(&sm, os);

    EXPECT_EQ(
        os.str(),
        "BinaryOpExpr at file: BinaryOpExpr.glu:1:3\n"
        "  -->plusOp Binary Operation with:\n"
        "    LiteralExpr at file: BinaryOpExpr.glu:1:1\n"
        "      -->Integer: 1\n"
        "    LiteralExpr at file: BinaryOpExpr.glu:1:5\n"
        "      -->Integer: 2\n"
    );
}
