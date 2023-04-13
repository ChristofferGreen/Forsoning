cmake_minimum_required(VERSION 3.14)
project(Forsoning LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#SET (CMAKE_CXX_FLAGS "-fuse-ld=mold")
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

find_package(Vulkan REQUIRED)

add_subdirectory("submods")
add_subdirectory("tests")
add_subdirectory("src")
