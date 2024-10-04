include (FetchContent)

FetchContent_Declare(
    LLVM
    GIT_REPOSITORY "https://github.com/llvm/llvm-project.git"
    GIT_TAG "llvmorg-18.1.8"
    GIT_SHALLOW 1
    SOURCE_SUBDIR llvm
    OVERRIDE_FIND_PACKAGE
)

set(LLVM_BUILD_LLVM_DYLIB ON)
set(LLVM_BUILD_DOCS OFF)
set(LLDB_INCLUDE_TESTS OFF)
set(LLVM_ENABLE_PROJECTS "clang;lldb")
set(LLVM_TARGETS_TO_BUILD "AArch64;X86")

FetchContent_MakeAvailable(LLVM)

include_directories(${llvm_BINARY_DIR}/include)
include_directories(${LLVM_SOURCE_DIR}/include)
