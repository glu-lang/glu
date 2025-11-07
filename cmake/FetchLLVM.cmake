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

set(LLVM_BUILD_LLVM_DYLIB ON)
set(LLVM_BUILD_DOCS OFF)
set(LLDB_INCLUDE_TESTS OFF)
set(LLVM_ENABLE_PROJECTS "clang;lldb")
set(LLVM_TARGETS_TO_BUILD "AArch64;X86")

FetchContent_MakeAvailable(LLVM)

include_directories(SYSTEM ${llvm_BINARY_DIR}/include)
include_directories(SYSTEM ${LLVM_SOURCE_DIR}/include)
