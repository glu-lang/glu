add_library (GILDec SHARED)

target_include_directories(GILDec
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GILDec
)

target_link_libraries(GILDec
    PUBLIC
    AST
    GIL
)

target_sources(GILDec
    PRIVATE
    GILDec.cpp
)
