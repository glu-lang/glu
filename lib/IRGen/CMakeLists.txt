add_library (IRGen SHARED)

target_include_directories(IRGen
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/IRGen
)

target_sources(IRGen
    PRIVATE
    IRGen.cpp
)
