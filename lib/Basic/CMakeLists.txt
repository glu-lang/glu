add_library (gluBasic SHARED)

target_include_directories(gluBasic
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(gluBasic
    PUBLIC
    LLVM
)

target_sources(gluBasic
    PRIVATE
    SourceManager.cpp
)
