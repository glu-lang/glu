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

if(FROM_SOURCE)
    # Ensure Clang's generated headers are available before compiling
    if(TARGET clang-tablegen-targets)
        add_dependencies(ClangImporter clang-tablegen-targets)
    endif()
    if(TARGET intrinsics_gen)
        add_dependencies(ClangImporter intrinsics_gen)
    endif()
endif()

target_sources(ClangImporter
    PRIVATE
    ClangImporter.cpp
    TypeConverter.cpp
    DeclImporter.cpp
)
