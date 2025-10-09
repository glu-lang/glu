# Build glu-demangle Rust tool
find_program(CARGO_EXECUTABLE cargo REQUIRED)

set(GLU_DEMANGLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GLU_DEMANGLE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Determine build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CARGO_BUILD_TYPE "release")
    set(CARGO_BUILD_FLAG "--release")
else()
    set(CARGO_BUILD_TYPE "debug")
    set(CARGO_BUILD_FLAG "")
endif()

# Output binary path
set(GLU_DEMANGLE_BINARY ${GLU_DEMANGLE_SOURCE_DIR}/target/${CARGO_BUILD_TYPE}/glu-demangle)

# Custom target to build glu-demangle
add_custom_target(glu-demangle ALL
    COMMAND ${CARGO_EXECUTABLE} build ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${GLU_DEMANGLE_SOURCE_DIR}
    COMMENT "Building glu-demangle Rust tool"
    BYPRODUCTS ${GLU_DEMANGLE_BINARY}
)

# Copy the binary to build/tools/glu-demangle
add_custom_command(TARGET glu-demangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${GLU_DEMANGLE_BINARY}
        ${CMAKE_BINARY_DIR}/tools/glu-demangle
    COMMENT "Copying glu-demangle to build/tools/"
)
