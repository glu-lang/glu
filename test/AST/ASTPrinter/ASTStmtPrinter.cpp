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
             << "    RefExpr " << lhs << " <line:1:1> -->Reference to: x\n"
             << "  -->ExprRight:\n"
                "    LiteralExpr "
             << rhs << " <line:1:5> -->Integer: 42\n";

    EXPECT_EQ(os.str(), expected.str());
}
