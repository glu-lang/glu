include (FetchContent)

if(NOT DEFINED GLU_LLVM_GIT_TAG)
    message(FATAL_ERROR "GLU_LLVM_GIT_TAG must be defined before including FetchLLVM.cmake")
endif()

FetchContent_Declare(
    LLVM
    GIT_REPOSITORY "https://github.com/llvm/llvm-project.git"
    GIT_TAG "${GLU_LLVM_GIT_TAG}"
    GIT_SHALLOW 1
    SOURCE_SUBDIR llvm
    OVERRIDE_FIND_PACKAGE
)

set(LLVM_BUILD_LLVM_DYLIB ON CACHE BOOL "" FORCE)
set(LLVM_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(LLDB_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_PROJECTS "clang;lldb" CACHE STRING "" FORCE)
set(LLVM_TARGETS_TO_BUILD "host" CACHE STRING "" FORCE)
if(ENABLE_ASAN)
    set(LLVM_USE_SANITIZER "Address" CACHE STRING "" FORCE)
endif()

message(STATUS "Fetching LLVM (tag: ${GLU_LLVM_GIT_TAG})")
FetchContent_MakeAvailable(LLVM)

# Use the correct paths based on how FetchContent structures the build
# Build directories first to ensure generated headers are found
set(LLVM_INCLUDE_DIRS
    ${llvm_BINARY_DIR}/include
    ${llvm_BINARY_DIR}/tools/clang/include
    ${llvm_SOURCE_DIR}/llvm/include
    ${llvm_SOURCE_DIR}/clang/include
)

include_directories(${LLVM_INCLUDE_DIRS})
