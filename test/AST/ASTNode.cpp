#include "ASTNode.hpp"
#include "Basic/SourceLocation.hpp"

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
    TestDeclBase(NodeKind kind, glu::SourceLocation loc) : DeclBase(kind, loc)
    {
    }
};

class TestStmtBase : public StmtBase {
public:
    TestStmtBase(NodeKind kind, glu::SourceLocation loc) : StmtBase(kind, loc)
    {
    }
};

class TestExprBase : public ExprBase {
public:
    TestExprBase(NodeKind kind, glu::SourceLocation loc) : ExprBase(kind, loc)
    {
    }
};

class TestASTNode : public ASTNode {
public:
    TestASTNode(NodeKind kind, glu::SourceLocation loc) : ASTNode(kind, loc) { }
};

TEST_F(ASTNodeTest, ASTNodeConstructor)
{
    TestASTNode node(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_EQ(node.getKind(), NodeKind::DeclBaseFirstKind);
}

TEST_F(ASTNodeTest, DeclBaseConstructor)
{
    TestDeclBase decl(NodeKind::FunctionDeclKind, loc);
    ASSERT_EQ(decl.getKind(), NodeKind::FunctionDeclKind);
    ASSERT_TRUE(llvm::isa<DeclBase>(&decl));
}

TEST_F(ASTNodeTest, StmtBaseConstructor)
{
    TestStmtBase stmt(NodeKind::ReturnStmtKind, loc);
    ASSERT_EQ(stmt.getKind(), NodeKind::ReturnStmtKind);
    ASSERT_TRUE(llvm::isa<StmtBase>(&stmt));
}

TEST_F(ASTNodeTest, ExprBaseConstructor)
{
    TestExprBase expr(NodeKind::BinaryOpExprKind, loc);
    ASSERT_EQ(expr.getKind(), NodeKind::BinaryOpExprKind);
    ASSERT_TRUE(llvm::isa<ExprBase>(&expr));
}

TEST_F(ASTNodeTest, DeclBaseClassof)
{
    TestDeclBase decl(NodeKind::FunctionDeclKind, loc);
    ASSERT_TRUE(llvm::isa<DeclBase>(&decl));
    ASSERT_FALSE(llvm::isa<StmtBase>(&decl));
    ASSERT_FALSE(llvm::isa<ExprBase>(&decl));
}

TEST_F(ASTNodeTest, StmtBaseClassof)
{
    TestStmtBase stmt(NodeKind::ReturnStmtKind, loc);
    ASSERT_TRUE(llvm::isa<StmtBase>(&stmt));
    ASSERT_FALSE(llvm::isa<DeclBase>(&stmt));
    ASSERT_FALSE(llvm::isa<ExprBase>(&stmt));
}

TEST_F(ASTNodeTest, ExprBaseClassof)
{
    TestExprBase expr(NodeKind::BinaryOpExprKind, loc);
    ASSERT_TRUE(llvm::isa<ExprBase>(&expr));
    ASSERT_FALSE(llvm::isa<DeclBase>(&expr));
    ASSERT_FALSE(llvm::isa<StmtBase>(&expr));
}
