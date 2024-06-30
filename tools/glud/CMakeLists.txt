add_executable (glud)

target_link_libraries(glud
    PRIVATE
    IRDec
    GILDec
    ASTPrinter
)

target_include_directories(glud
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)

target_sources(glud
    PRIVATE
    ./sources/main.cpp
)
