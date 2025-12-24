add_library(ClangImporter SHARED)

target_include_directories(ClangImporter
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/ClangImporter
)

# Link with Clang libraries
# Note: Clang libraries are found via find_package(Clang) in the main CMakeLists.txt
target_link_libraries(ClangImporter
    PUBLIC
    AST
    LLVM
    PRIVATE
    clangBasic
    clangLex
    clangParse
    clangAST
    clangSema
    clangFrontend
    clangSerialization
    clangTooling
)

target_sources(ClangImporter
    PRIVATE
    ClangImporter.cpp
    ImporterContext.cpp
    TypeConverter.cpp
    DeclImporter.cpp
)
