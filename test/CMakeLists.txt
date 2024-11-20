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
    gluBasic
    GTest::gtest_main
)

target_sources (unit_tests
    PRIVATE
    main.cpp
    AST/ASTNode.cpp
)

include (GoogleTest)
gtest_discover_tests (unit_tests)
