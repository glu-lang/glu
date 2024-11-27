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

    llvm::SmallVector<StmtBase *> stmts;
    CompoundStmt body(loc, nullptr, stmts);

    FunctionDecl decl(loc, nullptr, name, &type, std::move(params), &body);
    ASTNode *test = &decl;
    ASSERT_EQ(decl.getName(), name);
    ASSERT_EQ(decl.getType(), &type);
    ASSERT_EQ(decl.getBody(), &body);
    ASSERT_TRUE(llvm::isa<FunctionDecl>(test));
    ASSERT_FALSE(llvm::isa<StmtBase>(&decl));

    delete boolType;
}
