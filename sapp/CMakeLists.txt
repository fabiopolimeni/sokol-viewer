# select the 3D backend for sokol_gfx.h and sokol_app.h
add_definitions(-DSOKOL_NO_DEPRECATED)
if (FIPS_EMSCRIPTEN)
    add_definitions(-DSOKOL_GLES3)
elseif (FIPS_RASPBERRYPI)
    add_definitions(-DSOKOL_GLES2)
elseif (FIPS_ANDROID)
    add_definitions(-DSOKOL_GLES3)
elseif (SOKOL_USE_D3D11)
    add_definitions(-DSOKOL_D3D11)
elseif (SOKOL_USE_METAL)
    add_definitions(-DSOKOL_METAL)
else()
    if (FIPS_IOS)
        add_definitions(-DSOKOL_GLES3)
    else()
        add_definitions(-DSOKOL_GLCORE33)
    endif()
endif()

add_subdirectory(sokol)
add_subdirectory(ui)
add_subdirectory(tinyobjloader)

fips_begin_app(clear-sapp windowed)
    fips_vs_warning_level(3)
    fips_files(clear-sapp.c)
    fips_deps(sokol tinyobjloader mathc)
fips_end_app()

#-------------------------------------------------------------------------------
#   The same samples, but with debugging UI
#
fips_begin_app(clear-sapp-ui windowed)
    fips_vs_warning_level(3)
    fips_files(clear-sapp.c)
    fips_deps(sokol tinyobjloader mathc imgui dbgui)
fips_end_app()
target_compile_definitions(clear-sapp-ui PRIVATE USE_DBG_UI)
