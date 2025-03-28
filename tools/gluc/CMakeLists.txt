add_executable (gluc)

target_link_libraries(gluc PRIVATE
    AST
    ASTPrinter
    GIL
    gluBasic
    Lexer
    Parser
    LLVM
    LLVMSupport
)

target_include_directories(gluc
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)

target_sources(gluc
    PRIVATE
    ./sources/main.cpp
)
