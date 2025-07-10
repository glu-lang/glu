#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

#include "AST/ASTContext.hpp"
#include "AST/TypedASTWalker.hpp"

#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"

using namespace glu::ast;

struct Void { };

struct SimpleFoldVisitor
    : public TypedASTWalker<SimpleFoldVisitor, int, std::string, Void> {

    int postVisitExprBase([[maybe_unused]] ExprBase *node) { return -1; }
    int postVisitLiteralExpr(
        LiteralExpr *node, [[maybe_unused]] glu::types::TypeBase *type
    )
    {
        return std::get<llvm::APInt>(node->getValue()).getZExtValue();
    }
    int postVisitBinaryOpExpr(
        BinaryOpExpr *node, int lhs, [[maybe_unused]] int op, int rhs
    )
    {
        assert(
            node->getOperator()->getIdentifier() == "+"
            && "Unsupported operator"
        );
        return lhs + rhs;
    }
    std::string
    postVisitAssignStmt([[maybe_unused]] AssignStmt *node, int lhs, int rhs)
    {
        return "Assign " + std::to_string(lhs) + " = " + std::to_string(rhs);
    }
    std::string
    postVisitExpressionStmt([[maybe_unused]] ExpressionStmt *node, int expr)
    {
        return "Expression: " + std::to_string(expr);
    }
    std::string postVisitCompoundStmt(
        [[maybe_unused]] CompoundStmt *node, llvm::ArrayRef<std::string> stmts
    )
    {
        std::string result = "CompoundStmt {";
        for (auto const &stmt : stmts) {
            result += stmt + "; ";
        }
        return result + "}";
    }
};

TEST(TypedASTWalker, SimpleFoldVisitorExpr)
{
    SimpleFoldVisitor visitor;
    ASTContext ctx;
    auto &ast = ctx.getASTMemoryArena();

    // ast for (x + 3)
    BinaryOpExpr *node = ast.create<BinaryOpExpr>(
        glu::SourceLocation(1),
        ast.create<RefExpr>(
            glu::SourceLocation(1), NamespaceIdentifier { {}, "x" }
        ),
        ast.create<RefExpr>(
            glu::SourceLocation(1), NamespaceIdentifier { {}, "+" }
        ),
        ast.create<LiteralExpr>(
            llvm::APInt(32, 3),
            ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                glu::types::IntTy::Signed, 32
            ),
            glu::SourceLocation(2)
        )
    );

    EXPECT_EQ(visitor.visit(node), 2);
}

TEST(TypedASTWalker, SimpleFoldVisitorStmt)
{
    SimpleFoldVisitor visitor;
    ASTContext ctx;
    auto &ast = ctx.getASTMemoryArena();

    // ast for x = (7 + 3);
    AssignStmt *node = ast.create<AssignStmt>(
        glu::SourceLocation(1),
        ast.create<RefExpr>(
            glu::SourceLocation(1), NamespaceIdentifier { {}, "x" }
        ),
        glu::Token(glu::TokenKind::equalTok, "="),
        ast.create<BinaryOpExpr>(
            glu::SourceLocation(2),
            ast.create<LiteralExpr>(
                llvm::APInt(32, 7),
                ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                    glu::types::IntTy::Signed, 32
                ),
                glu::SourceLocation(2)
            ),
            ast.create<RefExpr>(
                glu::SourceLocation(1), NamespaceIdentifier { {}, "+" }
            ),
            ast.create<LiteralExpr>(
                llvm::APInt(32, 3),
                ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                    glu::types::IntTy::Signed, 32
                ),
                glu::SourceLocation(3)
            )
        )
    );

    EXPECT_EQ(visitor.visit(node), "Assign -1 = 10");
}

TEST(TypedASTWalker, SimpleFoldVisitorCompoundStmt)
{
    SimpleFoldVisitor visitor;
    ASTContext ctx;
    auto &ast = ctx.getASTMemoryArena();

    // ast for { x = 1; {}; 42; }
    CompoundStmt *node = ast.create<CompoundStmt>(
        glu::SourceLocation(1),
        llvm::SmallVector<StmtBase *> {
            ast.create<AssignStmt>(
                glu::SourceLocation(1),
                ast.create<RefExpr>(
                    glu::SourceLocation(1), NamespaceIdentifier { {}, "x" }
                ),
                glu::Token(glu::TokenKind::equalTok, "="),
                ast.create<LiteralExpr>(
                    llvm::APInt(32, 1),
                    ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                        glu::types::IntTy::Signed, 32
                    ),
                    glu::SourceLocation(2)
                )
            ),
            ast.create<CompoundStmt>(
                glu::SourceLocation(3), llvm::ArrayRef<StmtBase *> {}
            ),
            ast.create<ExpressionStmt>(
                glu::SourceLocation(4),
                ast.create<LiteralExpr>(
                    llvm::APInt(32, 42),
                    ctx.getTypesMemoryArena().create<glu::types::IntTy>(
                        glu::types::IntTy::Signed, 32
                    ),
                    glu::SourceLocation(4)
                )
            ) }
    );

    EXPECT_EQ(
        visitor.visit(node),
        "CompoundStmt {Assign -1 = 1; CompoundStmt {}; Expression: 42; }"
    );
}
