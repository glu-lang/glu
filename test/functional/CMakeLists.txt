# Configure lit.cfg with CMake variables
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.in
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg
    @ONLY
)
