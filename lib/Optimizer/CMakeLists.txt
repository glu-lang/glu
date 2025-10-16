add_library (Optimizer SHARED)

target_include_directories(Optimizer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Optimizer
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Optimizer
    PUBLIC
    GIL
    gluBasic
    PRIVATE
    GILGen
)

target_sources(Optimizer
    PRIVATE
    Optimizer.cpp
    PassManager.cpp
    PassManagerOptions.cpp
    GILPasses/VoidMainPass.cpp
    GILPasses/DeadCodeEliminationPass.cpp
    GILPasses/UnreachableInstChecker.cpp
    GILPasses/DropLoweringPass.cpp
)
