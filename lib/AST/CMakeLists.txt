add_library (AST SHARED)

target_include_directories(AST
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/AST
    ${PROJECT_SOURCE_DIR}/include/AST/Types
)

target_sources(AST
    PRIVATE
    AST.cpp
)
