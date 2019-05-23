# select the 3D backend for sokol_gfx.h and sokol_app.h
# the sokol_config() macro is in the top-level CMakeLists.txt file!
sokol_config()

add_definitions(-DSOKOL_NO_DEPRECATED)

add_subdirectory(atlas)
add_subdirectory(sokol)
add_subdirectory(ui)
add_subdirectory(tinyobjloader)

#-------------------------------------------------------------------------------
#   The viewer app with UI
#
fips_begin_app(viewer-sapp-ui windowed)
    fips_vs_warning_level(3)
    fips_files_ex(. viewer*.c NO_RECURSE)
    sokol_shader(shaders/geometry_pass.glsl ${slang})
    fips_deps(sokol tinyobjloader mathc cimgui sgui)
fips_end_app()
