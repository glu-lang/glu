# Set gluc path based on build environment
if(DOCKER_BUILD)
    set(GLUC_PATH "/usr/local/bin/gluc")
    set(GLUDEMANGLE_PATH "/usr/local/bin/glu-demangle")
    set(LLDB_CONF "/usr/bin/lldb -o \"settings set target.disable-aslr false\"")
else()
    # Prepend xcrun on macOS to find tools correctly
    if(APPLE)
        find_program(XCRUN_PATH xcrun)
    else()
        set(XCRUN_PATH "")
    endif()
    set(GLUC_PATH "${XCRUN_PATH} ${CMAKE_BINARY_DIR}/tools/gluc/gluc")
    set(GLUDEMANGLE_PATH "${XCRUN_PATH} ${CMAKE_BINARY_DIR}/tools/glu-demangle/glu-demangle")
    find_program(LLDB_CONF lldb)
endif()

# Configure lit.cfg with CMake variables
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.in
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg
    @ONLY
)
