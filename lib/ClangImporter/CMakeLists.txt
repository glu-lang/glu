add_library(ClangImporter SHARED)

target_include_directories(ClangImporter
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/ClangImporter
)

# Disable ASAN for ClangImporter due to incompatibility with Clang's BumpPtrAllocator
# This is a known issue where LLVM's allocator poisons/unpoisons memory in ways ASAN flags
target_compile_options(ClangImporter PRIVATE -fno-sanitize=address)
target_link_options(ClangImporter PRIVATE -fno-sanitize=address)

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
    TypeConverter.cpp
    DeclImporter.cpp
)
