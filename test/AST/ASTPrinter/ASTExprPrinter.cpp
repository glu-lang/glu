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
                "-->Operator: 'plusOp'\n"
                "  -->LeftOperand:\n"
                "    LiteralExpr "
             << lhs << " <line:1:1>\n"
             << "      -->Integer: 1\n"
             << "  -->RightOperand:\n"
                "    LiteralExpr "
             << rhs << " <line:1:5>\n"
             << "      -->Integer: 2\n";

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
                "  -->Callee:\n"
                "    RefExpr "
             << callee
             << " <line:1:1>\n"
                "      -->Reference to: foo\n"
                "  -->Args:\n"
                "    LiteralExpr "
             << arg1
             << " <line:1:5>\n"
                "      -->Integer: 42\n"
                "    RefExpr "
             << arg2
             << " <line:1:9>\n"
                "      -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintCastExpr)
{
    PREP_ASTPRINTER("cast<int>(x)", "CastExpr.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto value = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(10),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );

    auto node
        = ast.create<glu::ast::CastExpr>(glu::SourceLocation(0), value, &intTy);

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "CastExpr " << node << " <CastExpr.glu, line:1:1>\n"
             << "-->Casting to Int:\n"
             << "  -->CastedExpr:\n"
             << "    RefExpr " << value << " <line:1:11>\n"
             << "      -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintLiteralExpr)
{
    PREP_ASTPRINTER("42", "LiteralExpr.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto node = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(0)
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "LiteralExpr " << node << " <LiteralExpr.glu, line:1:1>\n"
             << "  -->Integer: 42\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintStructMemberExpr)
{
    PREP_ASTPRINTER("myStruct.member", "StructMemberExpr.glu");

    auto base = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(0),
        glu::ast::NamespaceIdentifier {
            std::array { llvm::StringRef("myStruct") }, "myStruct" }
    );

    auto node = ast.create<glu::ast::StructMemberExpr>(
        glu::SourceLocation(9), base, "member"
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "StructMemberExpr " << node
             << " <StructMemberExpr.glu, line:1:10>\n"
                "-->Member: member from struct:\n"
                "  -->StructExpr:\n"
                "    RefExpr "
             << base
             << " <line:1:1>\n"
                "      -->Reference to: myStruct\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintUnaryOpExpr)
{
    PREP_ASTPRINTER("-x", "UnaryOpExpr.glu");

    auto operand = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(1),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );

    auto node = ast.create<glu::ast::UnaryOpExpr>(
        glu::SourceLocation(0), operand,
        glu::Token(glu::TokenKind::subOpTok, "-")
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "UnaryOpExpr " << node
             << " <UnaryOpExpr.glu, line:1:1>\n"
                "-->Operator: 'subOp'\n"
                "  -->Operand:\n"
                "    RefExpr "
             << operand
             << " <line:1:2>\n"
                "      -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintTernaryConditionalExpr)
{
    PREP_ASTPRINTER("x > 0 ? x : -x", "TernaryConditionalExpr.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto condition = ast.create<glu::ast::BinaryOpExpr>(
        glu::SourceLocation(0),
        ast.create<glu::ast::RefExpr>(
            glu::SourceLocation(0),
            glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                            "x" }
        ),
        glu::Token(glu::TokenKind::gtOpTok, ">"),
        ast.create<glu::ast::LiteralExpr>(
            llvm::APInt(32, 0), &intTy, glu::SourceLocation(4)
        )
    );

    auto trueExpr = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(8),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );

    auto falseExpr = ast.create<glu::ast::UnaryOpExpr>(
        glu::SourceLocation(12),
        ast.create<glu::ast::RefExpr>(
            glu::SourceLocation(13),
            glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                            "x" }
        ),
        glu::Token(glu::TokenKind::subOpTok, "-")
    );

    auto node = ast.create<glu::ast::TernaryConditionalExpr>(
        glu::SourceLocation(0), condition, trueExpr, falseExpr
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "TernaryConditionalExpr " << node
             << " <TernaryConditionalExpr.glu, line:1:1>\n"
                "  -->Condition:\n"
                "    BinaryOpExpr "
             << condition
             << " <line:1:1>\n"
                "    -->Operator: 'gtOp'\n"
                "      -->LeftOperand:\n"
                "        RefExpr "
             << condition->getLeftOperand()
             << " <line:1:1>\n"
                "          -->Reference to: x\n"
                "      -->RightOperand:\n"
                "        LiteralExpr "
             << condition->getRightOperand()
             << " <line:1:5>\n"
                "          -->Integer: 0\n"
                "  -->TrueExpr:\n"
                "    RefExpr "
             << trueExpr
             << " <line:1:9>\n"
                "      -->Reference to: x\n"
                "  -->FalseExpr:\n"
                "    UnaryOpExpr "
             << falseExpr
             << " <line:1:13>\n"
                "    -->Operator: 'subOp'\n"
                "      -->Operand:\n"
                "        RefExpr "
             << falseExpr->getOperand()
             << " <line:1:14>\n"
                "          -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}
