add_library (AST SHARED)

target_include_directories(AST
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/AST
)

target_sources(AST
    PRIVATE
    AST.cpp
)
