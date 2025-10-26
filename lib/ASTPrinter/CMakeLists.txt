add_library (ASTPrinter SHARED)

target_include_directories(ASTPrinter
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/ASTPrinter
)

target_link_libraries(ASTPrinter
    PUBLIC
    AST
)

target_sources(ASTPrinter
    PRIVATE
    ASTPrinter.cpp
    CodePrinter.cpp
)
