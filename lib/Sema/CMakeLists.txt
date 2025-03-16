add_library (Sema SHARED)

target_include_directories(Sema
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Sema
)

target_link_libraries(Sema
    PUBLIC
    AST
)

target_sources(Sema
    PRIVATE
    Resolver.cpp
)
