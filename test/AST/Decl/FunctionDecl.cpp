#include "Decl/FunctionDecl.hpp"
#include "ASTContext.hpp"

#include <Expr/LiteralExpr.hpp>
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
    ASTContext ctx;
    llvm::StringRef name = "foo";

    auto boolType = ctx.getTypesMemoryArena().create<glu::types::BoolTy>();
    glu::types::TypeBase *returnType = boolType;

    std::vector<glu::types::TypeBase *> parameters = { boolType, boolType };
    auto arg1
        = ctx.getASTMemoryArena().create<LiteralExpr>(true, boolType, loc);
    auto arg2
        = ctx.getASTMemoryArena().create<LiteralExpr>(false, boolType, loc);

    auto funcTy = ctx.getTypesMemoryArena().create<glu::types::FunctionTy>(
        parameters, returnType
    );
    llvm::SmallVector<ParamDecl> params = {
        ParamDecl(loc, "a", boolType, arg1),
        ParamDecl(loc, "b", boolType, arg2),
    };

    auto const func = ctx.getASTMemoryArena().create<FunctionDecl>(
        loc, nullptr, name, funcTy, std::move(params)
    );

    ASSERT_EQ(func->getName(), name);
    ASSERT_EQ(func->getType(), funcTy);
    ASSERT_EQ(func->getParams().size(), 2);
    ASSERT_TRUE(func->getBody()->getStmts().empty());
    ASSERT_TRUE(llvm::isa<FunctionDecl>(func));
    ASSERT_FALSE(llvm::isa<StmtBase>(func));
}
