#include "AST/Stmts.hpp"
#include "ASTPrinter.hpp"

TEST_F(ASTPrinterTest, PrintAssignStmt)
{
    PREP_ASTPRINTER("x = 42", "AssignStmt.glu");

    glu::Token assignToken(glu::TokenKind::equalTok, "=");
    auto lhs = ctx.getASTMemoryArena().create<glu::ast::RefExpr>(
        glu::SourceLocation(0),
        glu::ast::NamespaceIdentifier { std::array { llvm::StringRef("x") },
                                        "x" }
    );
    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    auto rhs = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(4)
    );
    glu::ast::AssignStmt node(glu::SourceLocation(2), lhs, assignToken, rhs);

    node.print(&sm, os);

    EXPECT_EQ(
        os.str(),
        "AssignStmt at file: AssignStmt.glu:1:3\n"
        "  -->equal Assignement with:\n"
        "    RefExpr at file: AssignStmt.glu:1:1\n"
        "      -->Reference to: x\n"
        "    LiteralExpr at file: AssignStmt.glu:1:5\n"
        "      -->Integer: 42\n"
    );
}
