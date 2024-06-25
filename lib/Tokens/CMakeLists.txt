add_library (Tokens SHARED)

target_include_directories(Tokens
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/Tokens
)

target_sources(Tokens
    PRIVATE
    Tokens.cpp
)
