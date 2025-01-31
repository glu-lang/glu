add_library (Lexer SHARED)

target_include_directories(Lexer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Lexer
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

FLEX_TARGET(Scanner Scanner.l  ${CMAKE_CURRENT_BINARY_DIR}/Scanner.cpp)

target_link_libraries(Lexer
    PUBLIC
    gluBasic
    LLVM
    #   ${FLEX_LIBRARIES}
)

target_sources(Lexer
    PRIVATE
    ${FLEX_Scanner_OUTPUTS}
)
