#include <Types/TypeVisitor.hpp>
#include <Types/Types.hpp>
#include <gtest/gtest.h>

// Example class to test the visitor pattern for types
class TestVisitor : public glu::types::TypeVisitor<TestVisitor, void> {
public:
    std::string visitIntTy(glu::types::TypeBase *type)
    {
        return "Visiting IntType";
    }

    std::string visitBoolTy(glu::types::TypeBase *type)
    {
        return "Visiting BoolType";
    }

    std::string visitCharTy(glu::types::TypeBase *type)
    {
        return "Visiting CharType";
    }
};

// Tests the IntTy visit function
TEST(TestVisitor, VisitIntTy)
{
    TestVisitor visitor;
    glu::types::IntTy intType(glu::types::IntTy::Signed, 4);

    EXPECT_EQ(visitor.visitIntTy(&intType), "Visiting IntType");
}

// Tests the BoolTy visit function
TEST(TestVisitor, VisitBoolTy)
{
    TestVisitor visitor;
    glu::types::BoolTy boolType;

    EXPECT_EQ(visitor.visitBoolTy(&boolType), "Visiting BoolType");
}

// Tests the CharTy visit function
TEST(TestVisitor, VisitCharTy)
{
    TestVisitor visitor;
    glu::types::CharTy charType;

    EXPECT_EQ(visitor.visitCharTy(&charType), "Visiting CharType");
}
