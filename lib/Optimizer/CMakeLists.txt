add_library (Optimizer SHARED)

target_include_directories(Optimizer
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Optimizer
)

target_sources(Optimizer
    PRIVATE
    Optimizer.cpp
)
