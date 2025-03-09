find_package(BISON 3.2 REQUIRED)

BISON_TARGET(Parser
    Parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Parser.tab.hpp
)

add_library (Parser SHARED
    ${BISON_Parser_OUTPUTS}
)

target_include_directories(Parser
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Parser
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Parser
    PUBLIC
    gluBasic
    Lexer
    AST
)
