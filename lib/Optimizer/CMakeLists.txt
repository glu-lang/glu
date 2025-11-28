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

    # Transformation passes
    TransformationPasses/VoidMainPass.cpp
    TransformationPasses/DeadCodeEliminationPass.cpp
    TransformationPasses/UnreachableInstChecker.cpp
    TransformationPasses/DropLoweringPass.cpp
    # TransformationPasses/CopyLoweringPass.cpp
    TransformationPasses/DetectUninitializedPass.cpp
    TransformationPasses/EraseCopyOnStructExtractPass.cpp

    # Analysis passes
    AnalysisPasses/ValueUseChecker.cpp

)
