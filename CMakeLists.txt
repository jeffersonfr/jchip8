cmake_minimum_required (VERSION 3.0)

project (jatari)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")

find_package(PkgConfig REQUIRED)

pkg_check_modules(JLIBCPP REQUIRED jlibcpp)

add_definitions(${JLIBCPP_CFLAGS})

add_subdirectory(src)
