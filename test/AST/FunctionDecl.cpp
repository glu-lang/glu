#include "FunctionDecl.hpp"

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
    std::vector<glu::types::TypeBase *> parameters
        = { new glu::types::BoolTy(), new glu::types::BoolTy() };
    glu::types::TypeBase *returnType = new glu::types::BoolTy();

    glu::types::FunctionTy type(parameters, returnType);
    llvm::SmallVector<Param> params = { Param("a", new glu::types::BoolTy()),
                                        Param("b", new glu::types::BoolTy()) };
    TestDeclBase body(NodeKind::LetDeclKind, loc, nullptr);

    FunctionDecl decl(loc, nullptr, name, &type, std::move(params), &body);
    ASTNode *test = &decl;
    ASSERT_EQ(decl.getName(), name);
    ASSERT_EQ(decl.getType(), &type);
    ASSERT_EQ(decl.getBody(), &body);
    ASSERT_TRUE(llvm::isa<FunctionDecl>(test));
    ASSERT_FALSE(llvm::isa<StmtBase>(&decl));

    for (auto param : decl.getParams()) {
        delete param.type;
    }
    delete returnType;
    for (auto param : parameters) {
        delete param;
    }
}
