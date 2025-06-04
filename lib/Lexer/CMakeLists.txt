add_library(Lexer SHARED)

find_path(FLEX_INCLUDE_DIR FlexLexer.h REQUIRED)

find_package(FLEX REQUIRED)

FLEX_TARGET(Scanner Scanner.l ${CMAKE_CURRENT_BINARY_DIR}/Scanner.cpp)

target_sources(Lexer
        PRIVATE
        ${FLEX_Scanner_OUTPUTS}
)

target_include_directories(Lexer
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/Lexer
        ${FLEX_INCLUDE_DIR}
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Lexer
        PUBLIC
        gluBasic
        LLVM
)
