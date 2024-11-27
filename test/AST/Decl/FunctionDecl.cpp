#include "Decl/FunctionDecl.hpp"

#include <gtest/gtest.h>

using namespace glu::ast;

class FunctionDeclTest : public ::testing::Test {
protected:
    glu::SourceLocation loc;
    FunctionDeclTest() : loc(11) { }
};

class TestDeclBase : public DeclBase {
public:
    TestDeclBase(NodeKind kind, glu::SourceLocation loc, ASTNode *parent)
        : DeclBase(kind, loc, parent)
    {
    }
};

TEST_F(FunctionDeclTest, FunctionDeclConstructor)
{
    std::string name = "foo";
    auto boolType = new glu::types::BoolTy();
    std::vector<glu::types::TypeBase *> parameters = { boolType, boolType };
    glu::types::TypeBase *returnType = boolType;

    glu::types::FunctionTy type(parameters, returnType);
    llvm::SmallVector<Param> params
        = { Param("a", boolType), Param("b", boolType) };

    FunctionDecl decl(loc, nullptr, name, &type, std::move(params));
    ASTNode *test = &decl;
    ASSERT_EQ(decl.getName(), name);
    ASSERT_EQ(decl.getType(), &type);
    ASSERT_EQ(decl.getParams().size(), 2);
    ASSERT_TRUE(decl.getBody().getStmts().empty());
    ASSERT_TRUE(llvm::isa<FunctionDecl>(test));
    ASSERT_FALSE(llvm::isa<StmtBase>(&decl));

    delete boolType;
}
