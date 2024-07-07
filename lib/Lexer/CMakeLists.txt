add_library (Lexer SHARED)

target_include_directories(Lexer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Lexer
)

target_link_libraries(Lexer
    PUBLIC
    Tokens
)

target_sources(Lexer
    PRIVATE
    Lexer.cpp
)
