include (FetchContent)

FetchContent_Declare (LLVM
    GIT_REPOSITORY "https://github.com/llvm/llvm-project.git"
    GIT_TAG "llvmorg-18.1.8"
    GIT_SHALLOW 1
    SOURCE_SUBDIR "llvm/"
)

FetchContent_MakeAvailable(LLVM)
