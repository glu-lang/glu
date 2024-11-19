add_library (AST SHARED)

target_include_directories(AST
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(AST
    PUBLIC
        gluBasic
        LLVM
)

target_sources(AST
    PRIVATE
    AST.cpp
)
