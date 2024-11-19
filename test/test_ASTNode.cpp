#include "ASTNode.hpp"
#include "SourceLocation.hpp"

#include <llvm/Support/Casting.h>

#include <gtest/gtest.h>

using namespace glu::ast;

class ASTNodeTest : public ::testing::Test {
protected:
    glu::SourceLocation loc;
    ASTNodeTest() : loc(11) { }
};

class TestDeclBase : public DeclBase {
public:
    TestDeclBase(NodeKind kind, glu::SourceLocation loc, ASTNode *parent)
        : DeclBase(kind, loc, parent)
    {
    }
};

class TestStmtBase : public StmtBase {
public:
    TestStmtBase(NodeKind kind, glu::SourceLocation loc, ASTNode *parent)
        : StmtBase(kind, loc, parent)
    {
    }
};

class TestExprBase : public ExprBase {
public:
    TestExprBase(NodeKind kind, glu::SourceLocation loc, ASTNode *parent)
        : ExprBase(kind, loc, parent)
    {
    }
};

class TestASTNode : public ASTNode {
public:
    TestASTNode(NodeKind kind, glu::SourceLocation loc, ASTNode *parent)
        : ASTNode(kind, loc, parent)
    {
    }
};

TEST_F(ASTNodeTest, ASTNodeConstructor)
{
    TestASTNode node(NodeKind::DeclBaseFirstKind, loc, nullptr);
    ASSERT_EQ(node.getKind(), NodeKind::DeclBaseFirstKind);
}

TEST_F(ASTNodeTest, DeclBaseConstructor)
{
    TestDeclBase decl(NodeKind::FunctionDeclKind, loc, nullptr);
    ASSERT_EQ(decl.getKind(), NodeKind::FunctionDeclKind);
    ASSERT_TRUE(llvm::isa<DeclBase>(&decl));
}

TEST_F(ASTNodeTest, StmtBaseConstructor)
{
    TestStmtBase stmt(NodeKind::ReturnStmtKind, loc, nullptr);
    ASSERT_EQ(stmt.getKind(), NodeKind::ReturnStmtKind);
    ASSERT_TRUE(llvm::isa<StmtBase>(&stmt));
}

TEST_F(ASTNodeTest, ExprBaseConstructor)
{
    TestExprBase expr(NodeKind::BinaryOpExprKind, loc, nullptr);
    ASSERT_EQ(expr.getKind(), NodeKind::BinaryOpExprKind);
    ASSERT_TRUE(llvm::isa<ExprBase>(&expr));
}

TEST_F(ASTNodeTest, DeclBaseClassof)
{
    TestDeclBase decl(NodeKind::FunctionDeclKind, loc, nullptr);
    ASSERT_TRUE(llvm::isa<DeclBase>(&decl));
    ASSERT_FALSE(llvm::isa<StmtBase>(&decl));
    ASSERT_FALSE(llvm::isa<ExprBase>(&decl));
}

TEST_F(ASTNodeTest, StmtBaseClassof)
{
    TestStmtBase stmt(NodeKind::ReturnStmtKind, loc, nullptr);
    ASSERT_TRUE(llvm::isa<StmtBase>(&stmt));
    ASSERT_FALSE(llvm::isa<DeclBase>(&stmt));
    ASSERT_FALSE(llvm::isa<ExprBase>(&stmt));
}

TEST_F(ASTNodeTest, ExprBaseClassof)
{
    TestExprBase expr(NodeKind::BinaryOpExprKind, loc, nullptr);
    ASSERT_TRUE(llvm::isa<ExprBase>(&expr));
    ASSERT_FALSE(llvm::isa<DeclBase>(&expr));
    ASSERT_FALSE(llvm::isa<StmtBase>(&expr));
}
