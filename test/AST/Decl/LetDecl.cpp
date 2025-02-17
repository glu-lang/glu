#include "Decl/LetDecl.hpp"
#include "ASTNode.hpp"
#include "Types.hpp"

#include <gtest/gtest.h>

using namespace glu::ast;
using namespace glu::types;

class TestExprBase : public ExprBase {
public:
    TestExprBase() : ExprBase(NodeKind::LiteralExprKind, glu::SourceLocation(1))
    {
    }
};

TEST(LetDeclTest, Constructor)
{
    glu::SourceLocation loc(1);
    std::string name = "x";
    TypeBase *type = new BoolTy();
    ExprBase *value = new TestExprBase();

    LetDecl letDecl(loc, name, type, value);

    EXPECT_EQ(letDecl.getName(), name);
    EXPECT_EQ(letDecl.getType(), type);
    EXPECT_EQ(letDecl.getValue(), value);
    EXPECT_TRUE(llvm::isa<DeclBase>(letDecl));
    EXPECT_TRUE(llvm::isa<LetDecl>(letDecl));
    EXPECT_EQ(letDecl.getValue()->getParent(), &letDecl);

    delete type;
    delete value;
}
