#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

#include "AST/ASTContext.hpp"
#include "AST/ASTWalker.hpp"

#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"

using namespace glu::ast;

// Example class to test the visitor pattern for types
struct TestVisitor : public ASTWalker<TestVisitor> {
    int indent = -1;
    std::ostringstream acc;

    void beforeVisitNode([[maybe_unused]] ASTNode *node) { indent++; }
    void afterVisitNode([[maybe_unused]] ASTNode *node) { indent--; }
    void preVisitASTNode(ASTNode *node)
    {
        for (int i = 0; i < indent; ++i) {
            acc << "  ";
        }
        acc << "Visiting Node with Kind " << size_t(node->getKind()) << '\n';
    }
    void preVisitLiteralExpr([[maybe_unused]] ASTNode *node)
    {
        for (int i = 0; i < indent; ++i) {
            acc << "  ";
        }
        acc << "Visiting a Lit! " << '\n';
    }
};

TEST(ASTWalker, Example)
{
    TestVisitor visitor;
    ASTContext ctx;
    auto &ast = ctx.getASTMemoryArena();

    ASTNode *node = ast.create<IfStmt>(
        glu::SourceLocation(1),
        ast.create<LiteralExpr>(
            true, ctx.getTypesMemoryArena().create<glu::types::BoolTy>(),
            glu::SourceLocation(2)
        ),
        ast.create<CompoundStmt>(
            glu::SourceLocation(3),
            llvm::SmallVector<StmtBase *> {
                ast.create<AssignStmt>(
                    glu::SourceLocation(4),
                    ast.create<RefExpr>(
                        glu::SourceLocation(4), NamespaceIdentifier { {}, "x" }
                    ),
                    glu::Token(),
                    ast.create<LiteralExpr>(
                        llvm::APInt(32, 42),
                        ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                            glu::types::IntTy::Signed, 32
                        ),
                        glu::SourceLocation(4)
                    )
                ),
                ast.create<BreakStmt>(glu::SourceLocation(4)) }
        ),
        ast.create<CompoundStmt>(
            glu::SourceLocation(4), llvm::SmallVector<StmtBase *> {}
        )
    );

    visitor.visit(node);
    EXPECT_EQ(
        visitor.acc.str(),
        "Visiting Node with Kind 3\n"
        "  Visiting a Lit! \n"
        "  Visiting Node with Kind 10\n"
        "    Visiting Node with Kind 8\n"
        "      Visiting Node with Kind 17\n"
        "      Visiting a Lit! \n"
        "    Visiting Node with Kind 6\n"
        "  Visiting Node with Kind 10\n"
    );
}
