add_executable(glulsp
    main.cpp
    LSPServer.cpp
    LSPServer.hpp
    DocumentManager.cpp
    DocumentManager.hpp
    JSONRPCHandler.cpp
    JSONRPCHandler.hpp
)

# Get LLVM libraries
llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(glulsp 
    gluBasic
    AST
    Lexer
    Parser
    Sema
    ${llvm_libs}
)

target_include_directories(glulsp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${LLVM_INCLUDE_DIRS}
)

# Find and link nlohmann json library
find_package(nlohmann_json REQUIRED)
target_link_libraries(glulsp nlohmann_json::nlohmann_json)