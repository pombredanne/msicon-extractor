cmake_minimum_required (VERSION 2.6)
cmake_policy(VERSION 2.6)
project(msicon-extractor)


set(GETTEXT_PACKAGE "\"msicon-extractor\"")
set(VERSION 0.1)

# find glib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLib glib-2.0 REQUIRED)

# find libpng
find_package(PNG REQUIRED)

# we want c99
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic")

# sources
set(SOURCES_C
    src/write-icons.c
    src/extract-pe.c
    src/iconbmp2png.c
    src/extract-icon.c
    src/file.c
    src/main.c
    src/extract-common.c
)

include_directories(${GLib_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
link_directories(${GLib_LIBRARY_DIRS} ${PNG_LIBRARY_DIRS})
add_definitions(${GLib_CFLAGS_OTHER} -DGETTEXT_PACKAGE=${GETTEXT_PACKAGE})

add_executable(msicon-extractor ${SOURCES_C})
target_link_libraries(msicon-extractor ${GLib_LIBRARIES} ${PNG_LIBRARIES})

# testing
enable_testing()

set(SOURCES_C_TEST
    test/test.c
    test/util.c
)

# FIXME: the testing code needs to find its data at runtime, we just compile it in for convenience
add_definitions(
    -DEXTRACTOR_TEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/test/data\"
    -DEXTRACTOR_BINARY=\"${CMAKE_CURRENT_BINARY_DIR}/msicon-extractor\"
    -DEXTRACTOR_TEST_TMP=\"${CMAKE_CURRENT_BINARY_DIR}/test-tmp\"
)

add_executable(extractor-test ${SOURCES_C_TEST})
target_link_libraries(extractor-test ${GLib_LIBRARIES} ${PNG_LIBRARIES})
add_test(test ${CMAKE_CURRENT_BINARY_DIR}/extractor-test)