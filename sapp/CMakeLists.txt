# select the 3D backend for sokol_gfx.h and sokol_app.h
# the sokol_config() macro is in the top-level CMakeLists.txt file!
sokol_config()

add_definitions(-DSOKOL_NO_DEPRECATED)

fips_add_subdirectory(atlas)
fips_add_subdirectory(sokol)
fips_add_subdirectory(ui)
fips_add_subdirectory(tinyobjloader)
fips_add_subdirectory(cute)
fips_add_subdirectory(stb)
fips_add_subdirectory(containers)

#-------------------------------------------------------------------------------
#   The viewer app with UI
#
fips_begin_app(viewer-sapp-ui windowed)
if (FIPS_MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    fips_vs_warning_level(3)
endif()
    fips_files_ex(. viewer*.c NO_RECURSE)
    sokol_shader(shaders/geometry_pass.glsl ${slang})
    fips_deps(sokol tinyobjloader mathc imgui sgui stb cute containers)
fips_end_app()
