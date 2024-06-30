cmake_minimum_required (VERSION 3.29)

set(GLU_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${GLU_CMAKE_DIR})

# CMake will prevent in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD YES)

project(Glu C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(UnixCompileRules)
include(FetchLLVM)

# find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(./lib/Tokens/)
add_subdirectory(./lib/AST/)
add_subdirectory(./lib/GIL/)
add_subdirectory(./lib/Lexer/)
add_subdirectory(./lib/Parser/)
add_subdirectory(./lib/GILGen/)
add_subdirectory(./lib/Optimizer/)
add_subdirectory(./lib/IRGen/)

add_subdirectory(./lib/IRDec/)
add_subdirectory(./lib/GILDec/)
add_subdirectory(./lib/ASTPrinter/)

add_subdirectory(./tools/gluc/)
add_subdirectory(./tools/glud/)
