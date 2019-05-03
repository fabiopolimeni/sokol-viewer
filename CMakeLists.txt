#
# project: sokol-samples
#
cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup(PROJECT sokol-samples)

fips_import_fips_imgui_imgui()
fips_import_fips_mathc_mathc()

# just suppress this pesky "missing field initializer warning" for now
if (FIPS_CLANG OR FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
endif()
if (FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-braces")
endif()

add_subdirectory(sapp)

fips_finish()
