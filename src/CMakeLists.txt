cmake_minimum_required(VERSION 3.14)
project(Forsoning LANGUAGES CXX)

add_library(Forsoning INTERFACE)
target_include_directories(Forsoning INTERFACE "" "../ext")
