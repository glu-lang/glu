#include <Types/TypeVisitor.hpp>
#include <Types/Types.hpp>
#include <gtest/gtest.h>

// Example class to test the visitor pattern for types
class TestVisitor : public glu::types::TypeVisitor<TestVisitor, std::string> {
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

class TestVisitorFixture : public ::testing::Test {
protected:
    TestVisitor visitor;
};

// Tests the IntTy visit function
TEST_F(TestVisitorFixture, VisitIntTy)
{
    glu::types::IntTy intType(glu::types::IntTy::Signed, 4);

    EXPECT_EQ(visitor.visit(&intType), "Visiting IntType");
}

// Tests the BoolTy visit function
TEST_F(TestVisitorFixture, VisitBoolTy)
{
    TestVisitor visitor;
    glu::types::BoolTy boolType;

    EXPECT_EQ(visitor.visit(&boolType), "Visiting BoolType");
}

// Tests the CharTy visit function
TEST_F(TestVisitorFixture, VisitCharTy)
{
    TestVisitor visitor;
    glu::types::CharTy charType;

    EXPECT_EQ(visitor.visit(&charType), "Visiting CharType");
}
