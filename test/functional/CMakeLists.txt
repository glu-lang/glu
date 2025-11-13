# Set gluc path based on build environment
if(DOCKER_BUILD)
    set(GLUC_PATH "/usr/local/bin/gluc")
    set(LLDB_CONF "/usr/bin/lldb -o \"settings set target.disable-aslr false\"")
else()
    set(GLUC_PATH "${CMAKE_BINARY_DIR}/tools/gluc/gluc")
    set(LLDB_CONF "/usr/bin/lldb")
endif()

# Configure lit.cfg with CMake variables
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.in
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg
    @ONLY
)
