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

#-------------------------------------------------------------------------------
#   The viewer app with UI
#
fips_begin_app(viewer-sapp-ui windowed)
    fips_vs_warning_level(3)
    fips_files(viewer-sapp.c)
    fips_deps(sokol tinyobjloader mathc imgui sgui)
fips_end_app()
