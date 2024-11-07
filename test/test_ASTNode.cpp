#include "ASTNode.hpp"
#include "SourceLocation.hpp"
#include <gtest/gtest.h>

using namespace glu::ast;

class ASTNodeTest : public ::testing::Test {
protected:
    glu::SourceLocation loc;
    ASTNodeTest() : loc(11) { }
};

TEST_F(ASTNodeTest, ASTNodeConstructor)
{
    ASTNode node(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_EQ(node.getKind(), NodeKind::DeclBaseFirstKind);
}

TEST_F(ASTNodeTest, DeclBaseConstructor)
{
    DeclBase decl(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_EQ(decl.getKind(), NodeKind::DeclBaseFirstKind);
    ASSERT_TRUE(DeclBase::classof(&decl));
}

TEST_F(ASTNodeTest, StmtBaseConstructor)
{
    StmtBase stmt(NodeKind::StmtBaseFirstKind, loc, nullptr);
    ASSERT_EQ(stmt.getKind(), NodeKind::StmtBaseFirstKind);
    ASSERT_TRUE(StmtBase::classof(&stmt));
}

TEST_F(ASTNodeTest, ExprBaseConstructor)
{
    ExprBase expr(NodeKind::ExprBaseFirstKind, loc, nullptr);
    ASSERT_EQ(expr.getKind(), NodeKind::ExprBaseFirstKind);
    ASSERT_TRUE(ExprBase::classof(&expr));
}

TEST_F(ASTNodeTest, DeclBaseClassof)
{
    DeclBase decl(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_TRUE(DeclBase::classof(&decl));
    ASTNode node(NodeKind::StmtBaseFirstKind, loc);
    ASSERT_FALSE(DeclBase::classof(&node));
}

TEST_F(ASTNodeTest, StmtBaseClassof)
{
    StmtBase stmt(NodeKind::StmtBaseFirstKind, loc, nullptr);
    ASSERT_TRUE(StmtBase::classof(&stmt));
    ASTNode node(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_FALSE(StmtBase::classof(&node));
}

TEST_F(ASTNodeTest, ExprBaseClassof)
{
    ExprBase expr(NodeKind::ExprBaseFirstKind, loc, nullptr);
    ASSERT_TRUE(ExprBase::classof(&expr));
    ASTNode node(NodeKind::DeclBaseFirstKind, loc);
    ASSERT_FALSE(ExprBase::classof(&node));
}
