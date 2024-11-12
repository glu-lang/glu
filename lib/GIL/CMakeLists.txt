add_library (GIL SHARED)

target_include_directories(GIL
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GIL
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(GIL
    PUBLIC
    LLVM
    AST
    gluBasic
)

target_sources(GIL
    PRIVATE
    BasicBlock.cpp
    Function.cpp
    GIL.cpp
    InstBase.cpp
)
