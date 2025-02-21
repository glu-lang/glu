#include "AST/ASTContext.hpp"
#include "AST/ASTNode.hpp"
#include "AST/Decls.hpp"
#include "AST/Exprs.hpp"
#include "AST/Stmts.hpp"
#include "ASTPrinter.hpp"
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

namespace glu::ast {

class ASTPrinterTest : public ::testing::Test {
protected:
    std::string str;
    llvm::raw_string_ostream os;
    ASTPrinter printer;
    ASTContext context;

    ASTPrinterTest() : os(str), printer(os), context() { }
};

TEST_F(ASTPrinterTest, PrintEnumDecl)
{
    llvm::SmallVector<glu::types::EnumTy::Case> enumMembers
        = { glu::types::EnumTy::Case("Red", llvm::APInt(32, 0)),
            glu::types::EnumTy::Case("Green", llvm::APInt(32, 1)),
            glu::types::EnumTy::Case("Blue", llvm::APInt(32, 2)) };
    EnumDecl enumDecl(
        context, SourceLocation(42), nullptr, "Color", enumMembers
    );

    printer.visit(&enumDecl);
    EXPECT_EQ(
        str,
        "EnumDecl at loc : 42\nName: Color; Members : Red = 0, Green = 1, Blue "
        "= 2\n"
    );
}

TEST_F(ASTPrinterTest, PrintStructDecl)
{
    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    glu::types::FloatTy floatTy(32);
    glu::types::StructTy::Field field1 = { "Age", &intTy };
    glu::types::StructTy::Field field2 = { "Height", &floatTy };
    llvm::SmallVector<glu::types::StructTy::Field> fields = { field1, field2 };

    StructDecl structDecl(
        context, SourceLocation(42), nullptr, "Person", fields
    );

    printer.visit(&structDecl);

    EXPECT_EQ(
        str,
        "StructDecl at loc : 42\nName: Person; Fields : Age = IntTy, Height "
        "= FloatTy\n"
    );
}

} // namespace glu::ast
