add_executable (gluc)

target_link_libraries(gluc PRIVATE
    AST
    ASTPrinter
    GIL
    Sema
    GILGen
    IRGen
    gluBasic
    Lexer
    Parser
    LLVM
    Optimizer
)

target_include_directories(gluc
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)

target_sources(gluc
    PRIVATE
    ./sources/CompilerDriver.cpp
    ./sources/main.cpp
)
