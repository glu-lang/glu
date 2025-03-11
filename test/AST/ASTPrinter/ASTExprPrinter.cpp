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
    auto node = ast.create<glu::ast::BinaryOpExpr>(
        glu::SourceLocation(2), lhs, plusToken, rhs
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "BinaryOpExpr " << node
             << " <BinaryOpExpr.glu, line:1:3>\n"
                "  -->plusOp Binary Operation with:\n"
                "    LiteralExpr "
             << lhs
             << " <line:1:1> -->Integer: 1\n"
                "    LiteralExpr "
             << rhs << " <line:1:5> -->Integer: 2\n";

    EXPECT_EQ(os.str(), expected.str());
}
