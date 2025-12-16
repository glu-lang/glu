cmake_minimum_required (VERSION 3.29)

set(GLU_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${GLU_CMAKE_DIR})

# CMake will prevent in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD YES)

set(CMAKE_C_COMPILER clang CACHE STRING "C compiler to use")
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler to use")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Glu CXX C)

set(GLU_LLVM_VERSION "20" CACHE STRING "LLVM major version to use (supported: 19, 20)")

if(GLU_LLVM_VERSION STREQUAL "19")
    set(GLU_LLVM_VERSION_REQUIRED "19.1")
    set(GLU_LLVM_GIT_TAG "llvmorg-19.1.7")
elseif(GLU_LLVM_VERSION STREQUAL "20")
    set(GLU_LLVM_VERSION_REQUIRED "20.1")
    set(GLU_LLVM_GIT_TAG "llvmorg-20.1.8")
else()
    message(FATAL_ERROR "Unsupported LLVM version '${GLU_LLVM_VERSION}'. Supported versions: 19, 20")
endif()

message(STATUS "Configuring Glu with LLVM ${GLU_LLVM_VERSION} (find_package >= ${GLU_LLVM_VERSION_REQUIRED})")

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

option(FROM_SOURCE "Use LLVM built from source" OFF)

if(ENABLE_ASAN AND NOT FROM_SOURCE)
    message(FATAL_ERROR "ENABLE_ASAN can only be used when building LLVM from source.")
endif()

if(FROM_SOURCE)
    include(FetchLLVM)
    # Disable C++20 module dependency scanning - we don't use them
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
else()
    find_package(LLVM ${GLU_LLVM_VERSION_REQUIRED} REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    # Find Clang libraries - use the same prefix as LLVM
    get_filename_component(LLVM_CMAKE_DIR ${LLVM_DIR} DIRECTORY)
    set(Clang_DIR "${LLVM_CMAKE_DIR}/clang")
    find_package(Clang REQUIRED CONFIG)
    message(STATUS "Found Clang")
endif()

include(UnixCompileRules)

find_package(FLEX)

if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    message(STATUS "Code coverage instrumentation enabled")
endif()

add_subdirectory(./lib/Basic/)

add_subdirectory(./lib/AST/)
add_subdirectory(./lib/GIL/)
add_subdirectory(./lib/Lexer/)
add_subdirectory(./lib/Parser/)
add_subdirectory(./lib/ClangImporter/)
add_subdirectory(./lib/Sema/)
add_subdirectory(./lib/GILGen/)
add_subdirectory(./lib/Optimizer/)
add_subdirectory(./lib/IRGen/)

add_subdirectory(./lib/IRDec/)
add_subdirectory(./lib/GILDec/)
add_subdirectory(./lib/ASTPrinter/)

add_subdirectory(./tools/gluc/)
add_subdirectory(./tools/glu-demangle/)
add_subdirectory(./stdlib/)

add_subdirectory(./test/)
