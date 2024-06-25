add_library (ASTPrinter SHARED)

target_include_directories(ASTPrinter
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/ASTPrinter
)

target_sources(ASTPrinter
    PRIVATE
    ASTPrinter.cpp
)
