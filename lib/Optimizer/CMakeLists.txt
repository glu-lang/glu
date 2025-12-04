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
    PassManager.cpp
    PassManagerOptions.cpp

    # Analysis utilities
    Analysis/ValueUseChecker.cpp

    # Early passes (mandatory, can diagnose, don't degrade debug info)
    EarlyPasses/VoidMainPass.cpp
    EarlyPasses/DeadCodeEliminationPass.cpp
    EarlyPasses/UnreachableInstChecker.cpp
    EarlyPasses/DetectUninitializedPass.cpp
    EarlyPasses/SROAPass.cpp

    # Optimization passes (optional in -O0, can degrade debug info)
    # OptimizationPasses/

    # Late passes (mandatory, simplify IRGen work)
    LatePasses/DropLoweringPass.cpp
    # LatePasses/CopyLoweringPass.cpp

)
