add_library (gluBasic SHARED)

target_include_directories(gluBasic
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Basic
)

target_link_libraries(gluBasic
    PUBLIC
    LLVM
)

target_sources(gluBasic
    PRIVATE
    SourceManager.cpp
)
