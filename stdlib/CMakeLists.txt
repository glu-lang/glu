# Function to compile a .glu module to .o using gluc
function(glu_compile_module module_name)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.o"
        COMMAND "${CMAKE_BINARY_DIR}/tools/gluc/gluc" "-c" "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.glu" -o "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.o"
        DEPENDS gluc "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.glu"
        COMMENT "Compiling ${module_name}.glu with gluc"
    )
endfunction()

# List of stdlib modules (without extension)
set(stdlib_modules glucinfo std)

# Copy explicit .glu files from stdlib to build/tools/lib/glu
set(stdlib_glu_files)
foreach(module IN LISTS stdlib_modules)
    list(APPEND stdlib_glu_files "${CMAKE_CURRENT_SOURCE_DIR}/${module}.glu")
endforeach()
file(COPY ${stdlib_glu_files} DESTINATION "${CMAKE_BINARY_DIR}/tools/lib/glu")

# Compile .glu files using the function
set(stdlib_obj_files)
foreach(module ${stdlib_modules})
    glu_compile_module(${module})
    list(APPEND stdlib_obj_files "${CMAKE_BINARY_DIR}/tools/lib/glu/${module}.o")
endforeach()
add_custom_target(stdlib ALL DEPENDS ${stdlib_obj_files})
add_dependencies(stdlib gluc)
