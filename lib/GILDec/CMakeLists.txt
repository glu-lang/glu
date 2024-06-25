add_library (GILDec SHARED)

target_include_directories(GILDec
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GILDec
)

target_sources(GILDec
    PRIVATE
    GILDec.cpp
)
