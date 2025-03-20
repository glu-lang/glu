#include "AST/Stmts.hpp"
#include "ASTPrinter.hpp"
#include <sstream>

TEST_F(ASTPrinterTest, PrintAssignStmt)
{
    PREP_ASTPRINTER("x = 42", "AssignStmt.glu");

    glu::Token assignToken(glu::TokenKind::equalTok, "=");
    auto lhs = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(0),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );
    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto rhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(4)
    );
    auto node = ast.create<glu::ast::AssignStmt>(
        glu::SourceLocation(2), lhs, assignToken, rhs
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "AssignStmt " << node
             << " <AssignStmt.glu, line:1:3>\n"
                "-->equal Assignement with:\n"
                "  -->ExprLeft:\n"
                "    RefExpr "
             << lhs
             << " <line:1:1>\n"
                "      -->Reference to: x\n"
                "  -->ExprRight:\n"
                "    LiteralExpr "
             << rhs
             << " <line:1:5>\n"
                "      -->Integer: 42\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintBreakStmt)
{
    PREP_ASTPRINTER("break;", "BreakStmt.glu");

    auto node = ast.create<glu::ast::BreakStmt>(glu::SourceLocation(0));

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "BreakStmt " << node << " <BreakStmt.glu, line:1:1>\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintContinueStmt)
{
    PREP_ASTPRINTER("continue;", "ContinueStmt.glu");

    auto node = ast.create<glu::ast::ContinueStmt>(glu::SourceLocation(0));

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "ContinueStmt " << node << " <ContinueStmt.glu, line:1:1>\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintCompoundStmt)
{
    PREP_ASTPRINTER("{ x = 42; break; }", "CompoundStmt.glu");

    glu::Token assignToken(glu::TokenKind::equalTok, "=");
    auto lhs = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(2),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );
    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto rhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(6)
    );
    auto assignStmt = ast.create<glu::ast::AssignStmt>(
        glu::SourceLocation(4), lhs, assignToken, rhs
    );

    auto breakStmt = ast.create<glu::ast::BreakStmt>(glu::SourceLocation(10));

    auto node = ast.create<glu::ast::CompoundStmt>(
        glu::SourceLocation(0),
        std::vector<glu::ast::StmtBase *> { assignStmt, breakStmt }
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "CompoundStmt " << node
             << " <CompoundStmt.glu, line:1:1>\n"
                "  -->Stmts:\n"
                "    AssignStmt "
             << assignStmt
             << " <line:1:5>\n"
                "    -->equal Assignement with:\n"
                "      -->ExprLeft:\n"
                "        RefExpr "
             << lhs << " <line:1:3>\n"
             << "          -->Reference to: x\n"
                "      -->ExprRight:\n"
                "        LiteralExpr "
             << rhs << " <line:1:7>\n"
             << "          -->Integer: 42\n"
                "    BreakStmt "
             << breakStmt << " <line:1:11>\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintIfStmt)
{
    PREP_ASTPRINTER("if (x > 0) { x = 1; } else { x = -1; }", "IfStmt.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto condition = ast.create<glu::ast::BinaryOpExpr>(
        glu::SourceLocation(4),
        ast.create<glu::ast::RefExpr>(
            glu::SourceLocation(3),
            glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                            "x" }
        ),
        glu::Token(glu::TokenKind::gtOpTok, ">"),
        ast.create<glu::ast::LiteralExpr>(
            llvm::APInt(32, 0), &intTy, glu::SourceLocation(7)
        )
    );

    auto thenBody = ast.create<glu::ast::CompoundStmt>(
        glu::SourceLocation(10),
        std::vector<glu::ast::StmtBase *> { ast.create<glu::ast::AssignStmt>(
            glu::SourceLocation(12),
            ast.create<glu::ast::RefExpr>(
                glu::SourceLocation(12),
                glu::ast::NamespaceIdentifier {
                    std::array { llvm::StringRef("x") }, "x" }
            ),
            glu::Token(glu::TokenKind::equalTok, "="),
            ast.create<glu::ast::LiteralExpr>(
                llvm::APInt(32, 1), &intTy, glu::SourceLocation(16)
            )
        ) }
    );

    auto elseBody = ast.create<glu::ast::CompoundStmt>(
        glu::SourceLocation(23),
        std::vector<glu::ast::StmtBase *> { ast.create<glu::ast::AssignStmt>(
            glu::SourceLocation(25),
            ast.create<glu::ast::RefExpr>(
                glu::SourceLocation(25),
                glu::ast::NamespaceIdentifier {
                    std::array { llvm::StringRef("x") }, "x" }
            ),
            glu::Token(glu::TokenKind::equalTok, "="),
            ast.create<glu::ast::UnaryOpExpr>(
                glu::SourceLocation(29),
                ast.create<glu::ast::LiteralExpr>(
                    llvm::APInt(32, 1), &intTy, glu::SourceLocation(30)
                ),
                glu::Token(glu::TokenKind::subOpTok, "-")
            )
        ) }
    );

    auto node = ast.create<glu::ast::IfStmt>(
        glu::SourceLocation(0), condition, thenBody, elseBody
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "IfStmt " << node
             << " <IfStmt.glu, line:1:1>\n"
                "  -->Condition:\n"
                "    BinaryOpExpr "
             << condition
             << " <line:1:5>\n"
                "    -->gtOp Binary Operation with:\n"
                "      -->LeftOperand:\n"
                "        RefExpr "
             << condition->getLeftOperand()
             << " <line:1:4>\n"
                "          -->Reference to: x\n"
                "      -->RightOperand:\n"
                "        LiteralExpr "
             << condition->getRightOperand()
             << " <line:1:8>\n"
                "          -->Integer: 0\n"
                "  -->Body:\n"
                "    CompoundStmt "
             << thenBody
             << " <line:1:11>\n"
                "      -->Stmts:\n";

    auto thenAssignStmt
        = static_cast<glu::ast::AssignStmt *>(thenBody->getStmts()[0]);
    expected << "        AssignStmt " << thenAssignStmt
             << " <line:1:13>\n"
                "        -->equal Assignement with:\n"
                "          -->ExprLeft:\n"
                "            RefExpr "
             << thenAssignStmt->getExprLeft()
             << " <line:1:13>\n"
                "              -->Reference to: x\n"
                "          -->ExprRight:\n"
                "            LiteralExpr "
             << thenAssignStmt->getExprRight()
             << " <line:1:17>\n"
                "              -->Integer: 1\n";

    expected << "  -->Else:\n"
                "    CompoundStmt "
             << elseBody
             << " <line:1:24>\n"
                "      -->Stmts:\n";

    auto elseAssignStmt
        = static_cast<glu::ast::AssignStmt *>(elseBody->getStmts()[0]);
    auto elseExprRight
        = static_cast<glu::ast::UnaryOpExpr *>(elseAssignStmt->getExprRight());
    expected << "        AssignStmt " << elseAssignStmt
             << " <line:1:26>\n"
                "        -->equal Assignement with:\n"
                "          -->ExprLeft:\n"
                "            RefExpr "
             << elseAssignStmt->getExprLeft()
             << " <line:1:26>\n"
                "              -->Reference to: x\n"
                "          -->ExprRight:\n"
                "            UnaryOpExpr "
             << elseExprRight
             << " <line:1:30>\n"
                "            -->subOp Unary Operation with:\n"
                "              -->Operand:\n"
                "                LiteralExpr "
             << elseExprRight->getOperand()
             << " <line:1:31>\n"
                "                  -->Integer: 1\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintExpressionStmt)
{
    PREP_ASTPRINTER("x + 1;", "ExpressionStmt.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto expr = ast.create<glu::ast::BinaryOpExpr>(
        glu::SourceLocation(0),
        ast.create<glu::ast::RefExpr>(
            glu::SourceLocation(0),
            glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                            "x" }
        ),
        glu::Token(glu::TokenKind::plusOpTok, "+"),
        ast.create<glu::ast::LiteralExpr>(
            llvm::APInt(32, 1), &intTy, glu::SourceLocation(4)
        )
    );

    auto node
        = ast.create<glu::ast::ExpressionStmt>(glu::SourceLocation(0), expr);

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "ExpressionStmt " << node
             << " <ExpressionStmt.glu, line:1:1>\n"
                "  -->Expr:\n"
                "    BinaryOpExpr "
             << expr
             << " <line:1:1>\n"
                "    -->plusOp Binary Operation with:\n"
                "      -->LeftOperand:\n"
                "        RefExpr "
             << expr->getLeftOperand()
             << " <line:1:1>\n"
                "          -->Reference to: x\n"
                "      -->RightOperand:\n"
                "        LiteralExpr "
             << expr->getRightOperand()
             << " <line:1:5>\n"
                "          -->Integer: 1\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintForStmt)
{
    PREP_ASTPRINTER("for (let i: int in 0..10) {}", "ForStmt.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto binding = ast.create<glu::ast::ForBindingDecl>(
        glu::SourceLocation(5), "i", &intTy
    );

    auto range = ast.create<glu::ast::BinaryOpExpr>(
        glu::SourceLocation(12),
        ast.create<glu::ast::LiteralExpr>(
            llvm::APInt(32, 0), &intTy, glu::SourceLocation(12)
        ),
        glu::Token(glu::TokenKind::rangeOpTok, ".."),
        ast.create<glu::ast::LiteralExpr>(
            llvm::APInt(32, 10), &intTy, glu::SourceLocation(15)
        )
    );

    auto body = ast.create<glu::ast::CompoundStmt>(
        glu::SourceLocation(18), std::vector<glu::ast::StmtBase *> {}
    );

    auto node = ast.create<glu::ast::ForStmt>(
        glu::SourceLocation(0), binding, range, body
    );

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "ForStmt " << node
             << " <ForStmt.glu, line:1:1>\n"
                "  -->Binding:\n"
                "    ForBindingDecl "
             << binding
             << " <line:1:6>\n"
                "  -->Range:\n"
                "    BinaryOpExpr "
             << range
             << " <line:1:13>\n"
                "    -->rangeOp Binary Operation with:\n"
                "      -->LeftOperand:\n"
                "        LiteralExpr "
             << range->getLeftOperand()
             << " <line:1:13>\n"
                "          -->Integer: 0\n"
                "      -->RightOperand:\n"
                "        LiteralExpr "
             << range->getRightOperand()
             << " <line:1:16>\n"
                "          -->Integer: 10\n"
                "  -->Body:\n"
                "    CompoundStmt "
             << body
             << " <line:1:19>\n"
                "      -->Stmts:\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintReturnStmt)
{
    PREP_ASTPRINTER("return x;", "ReturnStmt.glu");

    auto expr = ast.create<glu::ast::RefExpr>(
        glu::SourceLocation(7),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );

    auto node = ast.create<glu::ast::ReturnStmt>(glu::SourceLocation(0), expr);

    node->debugPrint(&sm, os);

    std::ostringstream expected;
    expected << "ReturnStmt " << node
             << " <ReturnStmt.glu, line:1:1>\n"
                "  -->ReturnExpr:\n"
                "    RefExpr "
             << expr
             << " <line:1:8>\n"
                "      -->Reference to: x\n";

    EXPECT_EQ(os.str(), expected.str());
}
