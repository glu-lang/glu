cmake_minimum_required (VERSION 3.29)

set(GLU_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${GLU_CMAKE_DIR})

# CMake will prevent in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD YES)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Glu CXX C)

option(FROM_SOURCE "Use LLVM built from source" OFF)

if(FROM_SOURCE)
    include(FetchLLVM)
else()
    find_package(LLVM 18.1 REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
endif()

include(UnixCompileRules)

find_package(FLEX)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

add_subdirectory(./lib/Basic/)

add_subdirectory(./lib/AST/)
add_subdirectory(./lib/GIL/)
add_subdirectory(./lib/Lexer/)
add_subdirectory(./lib/Parser/)
add_subdirectory(./lib/Sema/)
add_subdirectory(./lib/GILGen/)
add_subdirectory(./lib/Optimizer/)
add_subdirectory(./lib/IRGen/)

add_subdirectory(./lib/IRDec/)
add_subdirectory(./lib/GILDec/)
add_subdirectory(./lib/ASTPrinter/)

add_subdirectory(./tools/gluc/)
add_subdirectory(./stdlib/)

add_subdirectory(./test/)
