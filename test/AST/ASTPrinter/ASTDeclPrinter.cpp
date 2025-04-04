#include "ASTPrinter.hpp"

TEST_F(ASTPrinterTest, PrintEnumDecl)
{
    PREP_ASTPRINTER("enum MyEnum { CASE1 = 1, CASE2 = 2 }", "EnumDecl.glu");

    std::vector<glu::types::Case> cases
        = { { "CASE1", llvm::APInt(32, 1) }, { "CASE2", llvm::APInt(32, 2) } };
    auto node = ast.create<glu::ast::EnumDecl>(
        ctx, glu::SourceLocation(0), nullptr, "MyEnum", cases
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { node };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "EnumDecl.glu", decls, &sm
    );

    node->debugPrint(os);

    std::ostringstream expected;
    expected << "EnumDecl " << node << " <line:1:1>\n"
             << "  -->Name: MyEnum\n"
             << "  -->Members:\n"
             << "  |  CASE1 = 1\n"
             << "  |  CASE2 = 2\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintLetDecl)
{
    PREP_ASTPRINTER("let x: int = 42;", "LetDecl.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto value = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 42), &intTy, glu::SourceLocation(10)
    );

    auto node = ast.create<glu::ast::LetDecl>(
        glu::SourceLocation(0), "x", &intTy, value
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { node };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "LetDecl.glu", decls, &sm
    );
    node->debugPrint(os);

    std::ostringstream expected;
    expected << "LetDecl " << node << " <line:1:1>\n"
             << "  -->Name: x\n"
             << "  -->Type: Int\n"
             << "  -->Value:\n"
             << "    LiteralExpr " << value << " <line:1:11>\n"
             << "      -->Integer: 42\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintStructDecl)
{
    PREP_ASTPRINTER("struct MyStruct { int a; int b; }", "StructDecl.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);
    llvm::SmallVector<glu::types::Field, 2> fields;

    fields.push_back({ "a", &intTy });
    fields.push_back({ "b", &intTy });

    auto node = ast.create<glu::ast::StructDecl>(
        ctx, glu::SourceLocation(0), nullptr, "MyStruct", fields
    );

    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { node };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "StructDecl.glu", decls, &sm
    );
    node->debugPrint(os);

    std::ostringstream expected;
    expected << "StructDecl " << node << " <line:1:1>\n"
             << "  -->Name: MyStruct\n"
             << "  -->Fields:\n"
             << "  |  a : Int\n"
             << "  |  b : Int\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintTypeAliasDecl)
{
    PREP_ASTPRINTER("typealias MyAlias = int", "TypeAliasDecl.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    auto node = ast.create<glu::ast::TypeAliasDecl>(
        ctx, glu::SourceLocation(0), nullptr, "MyAlias", &intTy
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { node };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "TypeAliasDecl.glu", decls, &sm
    );
    node->debugPrint(os);

    std::ostringstream expected;
    expected << "TypeAliasDecl " << node << " <line:1:1>\n"
             << "  -->Name: MyAlias\n"
             << "  -->Type: Int\n";

    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintVarDecl)
{
    PREP_ASTPRINTER("var x: bool = true;", "VarDecl.glu");

    glu::types::BoolTy boolTy;

    auto value = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(1, 0), &boolTy, glu::SourceLocation(10)
    );
    auto node = ast.create<glu::ast::VarDecl>(
        glu::SourceLocation(0), "x", &boolTy, value
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { node };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "VarDecl.glu", decls, &sm
    );
    node->debugPrint(os);

    std::ostringstream expected;
    expected << "VarDecl " << node << " <line:1:1>\n"
             << "  -->Name: x\n"
             << "  -->Type: Bool\n"
             << "  -->Value:\n"
             << "    LiteralExpr " << value << " <line:1:11>\n"
             << "      -->Integer: 0\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintParamDecl)
{
    PREP_ASTPRINTER("function foo(param: bool) {}", "ParamDecl.glu");

    glu::types::BoolTy boolTy;
    auto value = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(1, 0), &boolTy, glu::SourceLocation(5)
    );
    auto param = ast.create<glu::ast::ParamDecl>(
        glu::SourceLocation(1), "param", &boolTy, value
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { param };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "ParamDecl.glu", decls, &sm
    );
    param->debugPrint(os);

    std::ostringstream expected;

    expected << "ParamDecl " << param << " <line:1:2>\n"
             << "  -->param : Bool\n"
             << "  -->Value:\n"
             << "    LiteralExpr " << value << " <line:1:6>\n"
             << "      -->Integer: 0\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintImportDecl)
{
    PREP_ASTPRINTER("import file::helloFromFile", "ImportDecl.glu");

    std::vector<llvm::StringRef> comps = { "file" };
    std::vector<llvm::StringRef> sels = { "helloFromFile" };
    glu::ast::ImportPath importPath = { comps, sels };

    auto importDecl = ast.create<glu::ast::ImportDecl>(
        glu::SourceLocation(0), nullptr, importPath
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { importDecl };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "ImportDecl.glu", decls, &sm
    );
    importDecl->debugPrint(os);

    std::ostringstream expected;

    expected << "ImportDecl " << importDecl << " <line:1:1>\n"
             << "  -->Module: file::{helloFromFile}\n";
    EXPECT_EQ(os.str(), expected.str());
}

TEST_F(ASTPrinterTest, PrintFunctionDecl)
{
    PREP_ASTPRINTER("func add(a: Int, b: Int) -> Int {}", "FunctionDecl.glu");

    glu::types::IntTy intTy(glu::types::IntTy::Signed, 32);

    std::vector<glu::types::TypeBase *> paramTypes = { &intTy, &intTy };

    auto funcTy = ctx.getTypesMemoryArena().create<glu::types::FunctionTy>(
        paramTypes, &intTy
    );

    auto litA = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 0), &intTy, glu::SourceLocation(2)
    );
    auto paramA = ast.create<glu::ast::ParamDecl>(
        glu::SourceLocation(2), "a", &intTy, litA
    );

    auto litB = ast.create<glu::ast::LiteralExpr>(
        llvm::APInt(32, 0), &intTy, glu::SourceLocation(3)
    );
    auto paramB = ast.create<glu::ast::ParamDecl>(
        glu::SourceLocation(3), "b", &intTy, litB
    );

    llvm::SmallVector<glu::ast::ParamDecl *, 2> params;
    params.push_back(paramA);
    params.push_back(paramB);

    auto body = ast.create<glu::ast::CompoundStmt>(
        glu::SourceLocation(10), std::vector<glu::ast::StmtBase *> {}
    );

    auto funcDecl = ast.create<glu::ast::FunctionDecl>(
        glu::SourceLocation(0), nullptr, "add", funcTy, params, body
    );
    llvm::SmallVector<glu::ast::DeclBase *, 1> decls = { funcDecl };
    ast.create<glu::ast::ModuleDecl>(
        glu::SourceLocation(0), "StructDecl.glu", decls, &sm
    );
    funcDecl->debugPrint(os);

    std::ostringstream expected;
    expected << "FunctionDecl " << funcDecl
             << " <line:1:1>\n"
                "  -->Name: add\n"
                "  -->Return Type: Int\n"
                "  -->Body:\n"
                "    CompoundStmt "
             << body
             << " <line:1:11>\n"
                "      -->Stmts:\n";

    EXPECT_EQ(os.str(), expected.str());
}
