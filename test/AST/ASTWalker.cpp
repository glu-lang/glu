#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

#include "AST/ASTContext.hpp"
#include "AST/ASTWalker.hpp"

#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"

using namespace glu::ast;

// Example class to test the visitor pattern for types
class TestVisitor : public ASTWalker<TestVisitor, TraversalOrder::PreOrder> {
    int indent = -1;

public:
    void beforeVisitNode(ASTNode *node) { indent++; }
    void afterVisitNode(ASTNode *node) { indent--; }
    void visitASTNode(ASTNode *node)
    {
        for (int i = 0; i < indent; ++i) {
            llvm::outs() << "  ";
        }
        llvm::outs() << "Visiting Node with Kind " << size_t(node->getKind())
                     << '\n';
    }
    void visitLiteralExpr(ASTNode *node)
    {
        for (int i = 0; i < indent; ++i) {
            llvm::outs() << "  ";
        }
        llvm::outs() << "Visiting a Lit! " << '\n';
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
                    ast.create<RefExpr>(glu::SourceLocation(4), "x"),
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
}
