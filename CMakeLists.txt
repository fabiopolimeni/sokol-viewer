#
# project: sokol-viewer
#
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup(PROJECT sokol-viewer)

fips_import_fips_imgui_imgui()
fips_import_fips_mathc_mathc()

# just suppress this pesky "missing field initializer warning" for now
if (FIPS_CLANG OR FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
endif()
if (FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-braces")
endif()

# macro to select the sokol-backend and shader dialects
macro(sokol_config)
    add_definitions(-DSOKOL_NO_DEPRECATED)
    if (FIPS_EMSCRIPTEN)
        add_definitions(-DSOKOL_GLES3)
        set(slang "glsl300es:glsl100")
    elseif (FIPS_RASPBERRYPI)
        add_definitions(-DSOKOL_GLES2)
        set(slang "glsl100")
    elseif (FIPS_ANDROID)
        add_definitions(-DSOKOL_GLES3)
        set(slang "glsl300es")
    elseif (SOKOL_USE_D3D11)
        add_definitions(-DSOKOL_D3D11)
        set(slang "hlsl5")
    elseif (SOKOL_USE_METAL)
        add_definitions(-DSOKOL_METAL)
        if (FIPS_IOS)
            set(slang "metal_ios")
        else()
            set(slang "metal_macos")
        endif()
    else()
        if (FIPS_IOS)
            add_definitions(-DSOKOL_GLES3)
            set(slang "glsl300es")
        else()
            add_definitions(-DSOKOL_GLCORE33)
            set(slang "glsl330")
        endif()
    endif()
endmacro()

fips_add_subdirectory(sapp)

fips_finish()
