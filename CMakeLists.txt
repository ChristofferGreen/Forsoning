cmake_minimum_required(VERSION 3.14)
project(Forsoning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

add_subdirectory("dependencies")
add_subdirectory("tests")
