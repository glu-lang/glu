add_library (GILGen SHARED)

target_include_directories(GILGen
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GILGen
)

target_link_libraries(GILGen
    PUBLIC
    AST
    GIL
)

target_sources(GILGen
    PRIVATE
    Context.cpp
    GILGenStmt.cpp
    TypeTranslator.cpp
)
