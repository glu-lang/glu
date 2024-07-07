add_library (IRGen SHARED)

target_include_directories(IRGen
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/IRGen
)

target_link_libraries(IRGen
    PUBLIC
    GIL
    LLVM
)

target_sources(IRGen
    PRIVATE
    IRGen.cpp
)
