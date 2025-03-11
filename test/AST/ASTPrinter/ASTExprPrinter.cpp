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

TEST_F(ASTPrinterTest, PrintCallExpr)
{
    PREP_ASTPRINTER("foo(42, x)", "CallExpr.glu");

    auto callee = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(0),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("foo") },
                                        "foo" }
    );

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto arg1 = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(4)
    );
    auto arg2 = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(8),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );

    std::array<glu::ast::ExprBase *, 2> args = { arg1, arg2 };
    auto node
        = ast.create<glu::ast::CallExpr>(glu::SourceLocation(3), callee, args);

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "CallExpr " << node
             << " <CallExpr.glu, line:1:4>\n"
                "  -->Call to:\n"
                "    RefExpr "
             << callee
             << " <line:1:1> -->Reference to: foo\n"
                "  -->Arguments:\n"
                "    LiteralExpr "
             << arg1
             << " <line:1:5> -->Integer: 42\n"
                "    RefExpr "
             << arg2 << " <line:1:9> -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}
