add_executable (gluc)

target_link_libraries(gluc PRIVATE
    AST
    ASTPrinter
    GIL
    Sema
    GILGen
    IRGen
    IRDec
    Optimizer
    gluBasic
    Lexer
    Parser
    LLVM
)

target_include_directories(gluc
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)

target_sources(gluc
    PRIVATE
    ./sources/CompilerDriver.cpp
    ./sources/main.cpp
)

if(FROM_SOURCE)
    # Create a symlink to Clang's resource directory so gluc can find built-in headers
    # Clang's GetResourcesPath computes: <executable_dir>/../lib/clang/<version>
    # If gluc is at build/tools/gluc/gluc, it looks for build/tools/lib/clang/<version>
    # So we create: build/tools/lib/clang -> build/_deps/llvm-build/lib/clang
    add_custom_command(TARGET gluc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/tools/lib"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${llvm_BINARY_DIR}/lib/clang"
            "${CMAKE_BINARY_DIR}/tools/lib/clang"
        COMMENT "Creating symlink to Clang resource directory"
    )
endif()
