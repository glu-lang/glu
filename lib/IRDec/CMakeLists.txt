add_library (IRDec SHARED)

target_include_directories(IRDec
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/IRDec
)

target_sources(IRDec
    PRIVATE
    IRDec.cpp
)
