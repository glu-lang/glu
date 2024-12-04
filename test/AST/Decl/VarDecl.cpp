#include "Decl/VarDecl.hpp"
#include "ASTNode.hpp"
#include "Types/Types.hpp"

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

class TestExprBase : public ExprBase {
public:
    TestExprBase()
        : ExprBase(NodeKind::LiteralExprKind, glu::SourceLocation(1), nullptr)
    {
    }
};

TEST(VarDeclTest, Constructor)
{
    glu::SourceLocation loc(1);
    std::string name = "x";
    TypeBase *type = new BoolTy();
    ExprBase *value = new TestExprBase();

    VarDecl varDecl(loc, name, type, value);

    EXPECT_EQ(varDecl.getName(), name);
    EXPECT_EQ(varDecl.getType(), type);
    EXPECT_EQ(varDecl.getValue(), value);
    EXPECT_TRUE(llvm::isa<DeclBase>(varDecl));
    EXPECT_TRUE(llvm::isa<VarDecl>(varDecl));
    EXPECT_EQ(varDecl.getValue()->getParent(), &varDecl);

    delete type;
    delete value;
}
