add_library (GIL SHARED)

target_include_directories(GIL
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/GIL
)

target_link_libraries(GIL
    PUBLIC
    LLVM
)

target_sources(GIL
    PRIVATE
    GIL.cpp
    InstBase.cpp
)
