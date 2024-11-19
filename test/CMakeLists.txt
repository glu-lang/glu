include(FetchGTest)

enable_testing ()

add_executable (unit_tests)

target_link_libraries (unit_tests
    PRIVATE
    GTest::gtest_main
    AST
)

target_sources (unit_tests
    PRIVATE
    main.cpp
    TypeVisitor.cpp
)

include (GoogleTest)
gtest_discover_tests (unit_tests)
