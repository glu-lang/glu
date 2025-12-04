# Function to compile a .glu module to .o using gluc
function(glu_compile_module module_name)
    get_filename_component(module_dir "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}" DIRECTORY)
    file(MAKE_DIRECTORY "${module_dir}")
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.glu"
        COMMAND cpp -traditional -C -P
                ${CMAKE_CURRENT_SOURCE_DIR}/${module_name}.glu
                ${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.glu
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${module_name}.glu
        COMMENT "Copying and preprocessing ${module_name}.glu to build directory"
    )

    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.o"
        COMMAND "${CMAKE_BINARY_DIR}/tools/gluc/gluc" "-c" "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.glu" -o "${CMAKE_BINARY_DIR}/tools/lib/glu/${module_name}.o"
        DEPENDS gluc ${stdlib_glu_files}
        COMMENT "Compiling ${module_name}.glu with gluc"
    )
endfunction()

# List of stdlib modules (without extension)
set(stdlib_modules
    glucinfo
    std
    defaultImports
    defaultImports/allocation
    defaultImports/assert
    defaultImports/file
    defaultImports/io
    defaultImports/operators
    defaultImports/range
    defaultImports/std
    defaultImports/string
    defaultImports/stringType)

# Copy explicit .glu files from stdlib to build/tools/lib/glu
set(stdlib_src_files)
set(stdlib_glu_files)
set(stdlib_obj_files)
foreach(module IN LISTS stdlib_modules)
    list(APPEND stdlib_src_files "${CMAKE_CURRENT_SOURCE_DIR}/${module}.glu")
    list(APPEND stdlib_glu_files "${CMAKE_BINARY_DIR}/tools/lib/glu/${module}.glu")
    list(APPEND stdlib_obj_files "${CMAKE_BINARY_DIR}/tools/lib/glu/${module}.o")
endforeach()

# Compile .glu files using the function
foreach(module ${stdlib_modules})
    glu_compile_module(${module})
endforeach()
add_custom_target(stdlib ALL DEPENDS ${stdlib_obj_files})
add_dependencies(stdlib gluc)
