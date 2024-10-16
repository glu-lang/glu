add_library (Parser SHARED)

target_include_directories(Parser
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Parser
)

target_link_libraries(Parser
    PUBLIC
    gluBasic
    AST
)

target_sources(Parser
    PRIVATE
    Parser.cpp
)
