add_library (GILGen SHARED)

target_include_directories(GILGen
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GILGen
)

target_sources(GILGen
    PRIVATE
    GILGen.cpp
)
