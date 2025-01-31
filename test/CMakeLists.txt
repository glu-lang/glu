include(FetchGTest)

enable_testing ()

add_executable (unit_tests)

target_compile_options (unit_tests
    PUBLIC
    -fprofile-instr-generate
    -fcoverage-mapping
)

target_link_options (unit_tests
    PUBLIC
    -fprofile-instr-generate
    -fcoverage-mapping
)

target_link_libraries (unit_tests
    PRIVATE
    AST
    GIL
    gluBasic
    Lexer
    GTest::gtest_main
)

target_sources (unit_tests
    PRIVATE
    main.cpp
    AST/ASTNode.cpp
    AST/Decl/FunctionDecl.cpp
    AST/Decl/LetDecl.cpp
    AST/Decl/VarDecl.cpp
    AST/Stmt/BreakStmt.cpp
    AST/Stmt/CompoundStmt.cpp
    AST/Stmt/ContinueStmt.cpp
    AST/Stmt/ExpressionStmt.cpp
    AST/Stmt/ReturnStmt.cpp
    AST/Stmt/WhileStmt.cpp
    AST/Types/TypeVisitor.cpp
    GIL/GILPrinter.cpp
    Scanner/Scanner.cpp
)

include (GoogleTest)
gtest_discover_tests (unit_tests)
